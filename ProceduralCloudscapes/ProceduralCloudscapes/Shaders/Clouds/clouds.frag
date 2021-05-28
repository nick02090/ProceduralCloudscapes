#version 460 core
//===============================================================================================
// INPUT 
//===============================================================================================

// Camera
uniform vec3 cameraPosition;
uniform vec2 resolution;
uniform mat4 inverseProjection;
uniform mat4 inverseView;

// Sun
uniform float sunAltitude; // from range [0.0, 1.0] where 0.0 is night and 1.0 is clear day
uniform float sunAzimuth; // from range [-1.0, 1.0] where 0.0 is in front and (-)1.0 is behind
uniform float sunIntensity;

// Noise textures
layout ( binding = 1 ) uniform sampler3D perlinWorleyTex;
layout ( binding = 2 ) uniform sampler3D worleyTex;

// Clouds
layout ( binding = 0 ) uniform sampler2D weatherMapTex;
uniform float globalCloudsCoverage = 0.3f;
uniform float globalCloudsDensity = 0.5f;
uniform bool isBaseShape = false;

// Rendering
uniform float renderDistance = 1e5f;
uniform float minTransmittance = 1e-3f;

// Lighting
uniform float beerCoeff = 1.0;
uniform bool isPowder = true;
uniform float powderCoeff = 5.0;
uniform float csi = 5.0f; // amount of extra intensity
uniform float cse = 20.0f; // exponent deciding how centralized around the sun extra intensity is
uniform vec3 cloudsColor = vec3(1.f);
uniform vec3 sunColorDay = vec3(1.0);
uniform vec3 sunColorSunset = vec3(0.5, 0.3, 0.1);

//===============================================================================================
// CONSTANTS
//===============================================================================================

// Math
const float PI = 3.14159265358979323846;
const float PI_2 = 1.57079632679489661923;
const float PI_4 = 0.785398163397448309616;

// Earth
const float earthRadius = 6360e3f;
const float atmosphereRadius = 6420e3f;

// Sun
const float sunAngularDiameter = 0.009250245; // deg2rad(0.53)

// Scattering
const int VIEW_RAY_SAMPLES = 256;
const int SUN_RAY_SAMPLES = 12;
const float cloudsScatteringIN = 0.5f;
const float cloudsScatteringOUT = 0.5f;

// Clouds
const float cloudHeightLOW = 8e3f;
const float cloudHeightHIGH = 15e3f;

const vec4 cloudGradientLOW = vec4(0.0, 0.07, 0.08, 0.15);
const vec4 cloudGradientMEDIUM = vec4(0.0, 0.2, 0.42, 0.6);
const vec4 cloudGradientHIGH = vec4(0.0, 0.08, 0.75, 0.98);

const float cloudBaseScale = 0.00001;
const vec3 cloudBaseWeights = vec3(0.625, 0.25, 0.125);

const float cloudDetailScale = 0.0005;
const vec3 cloudDetailWeights = vec3(0.625, 0.25, 0.125);

const float cloudWeatherScale = 0.00005;

//===============================================================================================
// STRUCTS
//===============================================================================================

struct ray {
	vec3 origin;
	vec3 direction;
};

struct planet {
	float radius;
	float aRadius; // atmosphere radius
};

struct sun {
	float altitude;
	float azimuth;
	float intensity;
	float angularDiameter;
	vec3 colorDay;
	vec3 colorSunset;
};

struct cloud {
	float heightMin;
	float heightMax;
};

//===============================================================================================
// METHODS (MATH)
//===============================================================================================

// Calculates clip space coordinate (NDC space)
vec3 computeClipSpaceCoord(ivec2 fragCoord){
	vec2 rayNDC = 2.0*vec2(fragCoord.xy)/resolution.xy - 1.0;
	return vec3(rayNDC, 1.0);
}

// Converts/Remaps a value from one range to another, where x is value to be remapped,
// original range is [Lo, Ho] and a new one is [Ln, Hn].
float remap(float x, float Lo, float Ho, float Ln, float Hn)
{
    return (((x - Lo) / (Ho - Lo)) * (Hn - Ln)) + Ln;
}

// Solves quadratic equation (f(x) = ax^2 + bx + c)
void solveQuadratic(float a, float b, float c, out float x0, out float x1) {
    // calculate the discriminant of the function (b^2 - 4ac)
    float discriminant = b * b - 4.0 * a * c;
    // if discriminant is less than zero there is no solution
    if (discriminant <= 0.0) {
		x0 = 1e32;
		x1 = 0.0;
		return;
	}
    // pre-calculate the square root of discriminant and 2a
    float discriminantROOT = sqrt(discriminant);
    float a2 = 2.0 * a;
    // calculate the final solution
    x0 = max(0.0, (-b - discriminantROOT)/(a2));
    x1 = (-b + discriminantROOT)/(a2);
}

// Calculates ray-sphere intersection with the assumption that the sphere is centered at the origin
void raySphereIntersection(ray ray, float sphereRadius, out float t0, out float t1) {
    // create quadratic equation arguments (f(x) = ax^2 + bx + c)
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(ray.origin, ray.direction);
    float c = dot(ray.origin, ray.origin) - sphereRadius * sphereRadius;
    // calculate the equation
    solveQuadratic(a, b, c, t0, t1);
}

//===============================================================================================
// METHODS (CLOUDS SHAPE)
//===============================================================================================

// Calculates where the ray intersects with the cloud layer
// Outputs the distances to the cloud layer (low and high border range) as well as the size of the layer
void rayCloudLayerIntersection(in ray ray, in cloud cloud, out float distToLayerLow, out float distToLayerHigh, out float layer)
{
	// prepare result data for ray-cloud_lower_layer intersection 
	float tc_min0, tc_min1;
    raySphereIntersection(ray, cloud.heightMin, tc_min0, tc_min1);
	// prepare result data for ray-cloud_higher_layer intersection
	float tc_max0, tc_max1;
	raySphereIntersection(ray, cloud.heightMax, tc_max0, tc_max1);
	// set initial layer size and distance to lower layer
	distToLayerLow = 0.0f;
    layer = tc_max1;
	// calculate layer size
    if (tc_max1 > 0 && tc_max0 > 0) {
        layer = min(tc_min0-tc_max0, tc_max1-tc_max0);
        distToLayerLow = tc_max0;
    } else if (tc_max1 > 0 && tc_max0 <= 0 && tc_min0 <= 0) {
        layer = tc_max1 - tc_min1;
        distToLayerLow = tc_min1;
    } else if (tc_max1 > 0 && tc_max0 <= 0 && tc_min0 > 0) {
        layer = tc_min0;
        distToLayerLow = 0.f;
    }
	// clamp distance to layer low range
	distToLayerLow = max(0.0, distToLayerLow);
	// calculate distance to layer top range
	distToLayerHigh = max(0.0, distToLayerLow + layer);
	// clamp the layer size
	layer = min(abs(distToLayerHigh - distToLayerLow), renderDistance);
}

// Gradient function that clamps the height value based on the cloud type
// When cloud type is 0.0 it will result into a low height clouds (stratus)
// When cloud type is 0.5 it will result into a medium height clouds (cumulus)
// When cloud type is 1.0 it will result into a high height clouds (cumulonimbus)
float calculateDensityHeightGradient(float height, float cloudType) {
	float a = 1.0 - clamp(cloudType * 2.0, 0.0, 1.0);
	float b = 1.0 - abs(cloudType - 0.5) * 2.0;
	float c = clamp(cloudType - 0.5, 0.0, 1.0) * 2.0;

	vec4 gradient = cloudGradientLOW * a + cloudGradientMEDIUM * b + cloudGradientHIGH * c;
	
	return smoothstep(gradient.x, gradient.y, height) - smoothstep(gradient.z, gradient.w, height);
}

// Position to relative height in cloud layer
float calculateCloudHeightFraction(vec3 position, cloud cloud) {
	return (position.y - cloud.heightMin) / (cloud.heightMax - cloud.heightMin);
}

// Calculates cloud height alterations (rounds the clouds towards the bottom and top)
float calculateCloudHeightAlteration(vec3 position, float cloudHeightFraction, cloud cloud, float weatherMapB) {
	float alterBottom = clamp(remap(cloudHeightFraction, 0, 0.07, 0.0, 1.0), 0.0, 1.0);
	float alterTop = clamp(remap(cloudHeightFraction, weatherMapB * 0.2, weatherMapB, 1.0, 0.0), 0.0, 1.0);
	return alterBottom * alterTop;
}

// Calculates cloud density alterations (fluffy at the bottom, defined shapes towards the top)
float calculateCloudDensityAlteration(vec3 position, float cloudHeightFraction, cloud cloud, float weatherMapA) {
	float alterBottom = cloudHeightFraction * clamp(remap(cloudHeightFraction, 0.0, 0.15, 0.0, 1.0), 0.0, 1.0);
	float alterTop = clamp(remap(cloudHeightFraction, 0.9, 1.0, 1.0, 0.0), 0.0, 1.0);
	return globalCloudsDensity * alterBottom * alterTop * 2.0 * weatherMapA;
}

// Projects position (3D) onto plane (2D) for texture loading
vec2 getProjection(vec3 position){
    vec3 sphereNormal = normalize(position);
    float u = atan(sphereNormal.x, sphereNormal.z) / (2 * PI) + 0.5;
    float v = asin(sphereNormal.y) / PI + 0.5;
	return vec2(u, v);
}

// Calculates cloud density (models cloud shape)
float calculateCloudDensity(vec3 position, bool isHighQuality, cloud cloud) {
	// load base shape texture (perlin-worley noise)
	vec4 base = texture(perlinWorleyTex, cloudBaseScale * position);
	float baseFBM = dot(base.gba, cloudBaseWeights);

	// load the weather map
	vec4 weatherMap = texture(weatherMapTex, getProjection(position) * cloudWeatherScale);
	// calculate weather map control
	float weatherMapControl = max(weatherMap.r, clamp(globalCloudsCoverage - 0.5, 0.0, 1.0) * weatherMap.g * 2.0);

	// calculate density with base noise
	float baseDensity = remap(base.r, baseFBM - 1.0, 1.0, 0.0, 1.0);

	// calculate cloud height fraction
	float cloudHeightFraction = calculateCloudHeightFraction(position, cloud);

	// calculate the density
	float density = remap(baseDensity * calculateCloudHeightAlteration(position, cloudHeightFraction, cloud, weatherMap.b), 1.0 - globalCloudsCoverage * weatherMapControl, 1.0, 0.0, 1.0);

	// sample extra detail noise on the edges if isHighQuality
	if (isHighQuality) {
		// load detail shape texture (worley32 noise)
		vec3 detail = texture(worleyTex, cloudDetailScale * position).rgb;
		float detailFBM = dot(detail, cloudDetailWeights);

		float cloudHeightFraction = calculateCloudHeightFraction(position, cloud);
		float densityModification = 0.35 * exp(- globalCloudsCoverage * 0.75) * mix(detailFBM, 1 - detailFBM, clamp(cloudHeightFraction * 0.5, 0.0, 1.0));

		density = remap(density, densityModification, 1.0, 0.0, 1.0);
	}

	// return clamped value
	return clamp(density, 0.0, 1.0) * calculateCloudDensityAlteration(position, cloudHeightFraction, cloud, weatherMap.a);
}

//===============================================================================================
// METHODS (CLOUDS LIGHTING)
//===============================================================================================

// Calculates attenuation of light for given cloud density based on Beer-Lambert's law
float calculateBeerLambert(float density) {
	return exp(- beerCoeff * density);
}

// Calculates powder effect for given cloud density
float calculatePowder(float density) {
	return 1.0 - calculateBeerLambert(density * powderCoeff);
}

// Calculates in-scattering (to create silver lining) based on Henyey-Greenstein phase function
float calculateHenyeyGreensteinPhase(float mu, float g) {
	float g2 = g * g;
	float mu2 = mu * mu;
    return 3.0 / (8.0 * PI) * ((1.0 - g2) * (1.0 + mu2) / ((2.0 + g2) * pow(1.0 + g2 - 2.0 * g * mu, 1.5)));
}

// Calculates cloud density (ray-march from cloud position to sun)
vec3 calculateCloudLight(ray view, vec3 position, vec3 sunDirection, float mu, cloud cloud, float cloudLayer, sun sun, vec3 lightColor) {
	// initialize variables for ray-marching
	vec3 color = vec3(0.0f);
	float transmittance = 1.0f;

	// calculate the sun ray segment length
	float segmentLength = cloudLayer / float(SUN_RAY_SAMPLES);

	// iterate over sun-ray direction
	for (int i = 0; i < SUN_RAY_SAMPLES; ++i) {
		// calculate current sample position
		vec3 samplePosition = position + segmentLength * sunDirection;
		// calculate density
		float density = calculateCloudDensity(samplePosition, !isBaseShape, cloud);

		// calculate the color if the density is above zero
		if (density > 0.0) {
			// calculate transmittance
			transmittance *= calculateBeerLambert(density * segmentLength);
			// accumulate final color
			color += density * segmentLength * transmittance;
		}

		// increase current position
		view.origin += segmentLength * sunDirection;
	}

	// calculate extra sun intensity (this is used to increase the HG effect)
	float extraSunIntensity = csi * clamp(pow(mu, cse), 0.0, 1.0) * (sunIntensity / 20.);
	
	// calculate scattering
	float inScattering = calculateHenyeyGreensteinPhase(mu, cloudsScatteringIN);
	float outScattering = calculateHenyeyGreensteinPhase(mu, -cloudsScatteringOUT);
	float scattering = mix(max(inScattering, extraSunIntensity), outScattering, 0.5);

	// return final color
	return color * scattering * lightColor + cloudsColor;
}

//===============================================================================================
// METHODS (CLOUDS)
//===============================================================================================

// Calculates the color for the clouds
vec4 clouds(in ray view, in planet earth, in cloud cloud, in sun sun, out float distanceToCloudLayer) 
{
	// prepare data for ray-cloud_layer intersections
	float distanceToCloudLow, distanceToCloudHigh, cloudLayer;
	// calculate ray-cloud_layer intersections
	rayCloudLayerIntersection(view, cloud, distanceToCloudLow, distanceToCloudHigh, cloudLayer);
	// calculate distance to cloud layer (for above, below and inside look of the clouds)
	distanceToCloudLayer = min(distanceToCloudLow, distanceToCloudHigh);

	// calculate the sun direction
	float cosSunAlt = cos(sun.altitude);
	vec3 sunDirection = vec3(cos(sun.azimuth) * cosSunAlt, sin(sun.altitude), sin(sun.azimuth) * cosSunAlt);
	
	// calculate the cosine of angle between the sun direction and the ray direction
	float mu = dot(view.direction, sunDirection);

	// initialize variables for ray-marching
	float distancePassed = 0.0f;
	vec3 color = vec3(0.0);
	float transmittance = 1.0f;

	// calculate number of steps in a way that its smaller number when looking directly in the sun
	float numberOfSteps = (1. - 0.5 * mu) * VIEW_RAY_SAMPLES;

	// calculate the view ray segment length
	float segmentLength = cloudLayer / numberOfSteps;

	// move ray origin to the intersection with cloud lower layer
	view.origin += view.direction * distanceToCloudLayer;
	// update the distance passed
	distancePassed += distanceToCloudLayer;

	// calculate light color
	float sigmoid = 1 / (1.0 + exp(8.0 - sunDirection.y * 40.0));
	float a = min(max(sigmoid, 0.0f), 1.0f);
	float b = 1.0 - a;
	vec3 lightColor = sun.colorDay * a + sun.colorSunset * b;

	// iterate over view-ray direction
	for (int i = 0; i < numberOfSteps; ++i) {
		// some early exit optimizations
		if (distancePassed > renderDistance) break;
		if (distanceToCloudLow == distanceToCloudHigh) break;
		if (transmittance < minTransmittance) break;

		// calculate current sample position
		vec3 samplePosition = view.origin + segmentLength * view.direction;
		// calculate base density for this position
		float baseDensity = calculateCloudDensity(samplePosition, false, cloud);

		if (baseDensity > 0.0) {
			// calculate high quality density
			float density = calculateCloudDensity(samplePosition, !isBaseShape, cloud);

			// calculate the color if the density is above zero
			if (density > 0.0) {

				// calculate transmittance
				transmittance *= calculateBeerLambert(density * segmentLength);

				// set default powder effect
				float powder = 1.0;
				if (isPowder) {
					// calculate powder effect
					powder = mix(calculatePowder(density * segmentLength), 1.0, mu);
				}
				
				// accumulate final color
				color += calculateCloudLight(view, samplePosition, sunDirection, mu, cloud, cloudLayer, sun, lightColor) * density * segmentLength * transmittance * powder * lightColor;
			}
		}

		// increase current position
		view.origin += segmentLength * view.direction;
		distancePassed += segmentLength;
	}

	// return final clouds color
	return vec4(color, transmittance);
}

//===============================================================================================
// MAIN
//===============================================================================================

void main()
{
	// Recalculate space
	ivec2 fragCoord = ivec2(gl_FragCoord);
	vec4 clipRay = vec4(computeClipSpaceCoord(fragCoord), 1.0);
	vec4 viewRay = inverseProjection * clipRay;
	viewRay = vec4(viewRay.xy, -1.0, 0.0);
	vec3 rd = normalize((inverseView * viewRay).xyz);

	// translate camera for earth radius
	vec3 ro = cameraPosition + vec3(0.0, earthRadius, 0.0);
	
	// create view ray
	ray view = ray(ro, rd);
	
	// prepare earth info
	planet earth = planet(earthRadius, atmosphereRadius);
	
	// prepare cloud info
	cloud cloud = cloud(earthRadius + cloudHeightLOW, earthRadius + cloudHeightHIGH);
	
	// calculate sun altitude and azimuth
	float sunAlt = 4.0 * - sunAngularDiameter + 1.6 * PI_4 * (0.5 + cos((1.0 - sunAltitude) * 3.0) / 2.0);
	float sunAzi = (1.0 - sunAzimuth * 0.7) * 4.6;

	// prepare sun info
    sun sun = sun(sunAlt, sunAzi, sunIntensity, sunAngularDiameter, sunColorDay, sunColorSunset);

	// prepare distance to clouds layer
	float distanceToCloudLayer;

	// calculate the clouds color
	vec4 clouds = clouds(view, earth, cloud, sun, distanceToCloudLayer);

	// calculate atmosphere amount for the clouds
	vec3 atmosphereColor = vec3(0.0, 0.0, 0.0); // atmosphere color should be black due to blending clouds with background texture
	float atmosphereAmount = (1.0f / renderDistance) * distanceToCloudLayer;

	// blend clouds with atmosphere
	vec4 result = mix(clouds, vec4(atmosphereColor, 1.0), atmosphereAmount);

	// output the final result
	gl_FragColor = result;
}