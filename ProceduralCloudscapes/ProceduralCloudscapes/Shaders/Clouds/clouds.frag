#version 460 core
//===============================================================================================
// INPUT 
//===============================================================================================

// Camera
uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform vec3 cameraUp;
uniform vec2 resolution;

// Sun
uniform float sunAltitude; // from range [0.0, 1.0] where 0.0 is night and 1.0 is clear day
uniform float sunAzimuth; // from range [-1.0, 1.0] where 0.0 is in front and (-)1.0 is behind
uniform float sunIntensity;
uniform vec3 sunColor = vec3(0.5, 0.3, 0.1);;

// Noise textures
uniform sampler3D perlinWorleyTex;
uniform sampler3D worleyTex;
uniform sampler2D curlTex;

// Clouds
uniform sampler2D weatherMapTex;
uniform float globalCloudsCoverage = 0.3f;
uniform float globalCloudsDensity = 0.5f;
uniform vec3 cloudsColor = vec3(1.f);

// Rendering
uniform float renderDistance = 1e5f;
uniform float minTransmittance = 1e-3f;

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

// Clouds
const float cloudHeightLOW = 5e3f;
const float cloudHeightHIGH = 12e3f;
const vec4 cloudGradientLOW = vec4(0.0, 0.07, 0.08, 0.15);
const vec4 cloudGradientMEDIUM = vec4(0.0, 0.2, 0.42, 0.6);
const vec4 cloudGradientHIGH = vec4(0.0, 0.08, 0.75, 0.98);
const float cloudBaseScale = 0.00001;
const vec3 cloudBaseWeights = vec3(0.625, 0.25, 0.125);
const float cloudDetailScale = 0.0002;
const vec3 cloudDetailWeights = vec3(0.625, 0.25, 0.125);
const float cloudDetailMultiplier = 0.05;

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
	vec3 color;
};

struct cloud {
	float heightMin;
	float heightMax;
};

//===============================================================================================
// METHODS
//===============================================================================================

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

// Calculates cloud density (models cloud shape)
float calculateCloudDensity(vec3 position, bool isHighQuality, cloud cloud) {
	// load base shape texture (perlin-worley noise)
	vec4 base = texture(perlinWorleyTex, cloudBaseScale * position);
	float baseFBM = dot(base.gba, cloudBaseWeights);

	// calculate density with base noise
	float density = remap(base.r, baseFBM - 1.0, 1.0, 0.0, 1.0);
	// TODO: 1.0 in calculateDensityHeightGradient(height, 1.0) is a cloudType and should be replaced with a weatherMap information
	density *= calculateDensityHeightGradient(calculateCloudHeightFraction(position, cloud), 1.0);
	
	// recalculate the density based on the global coverage
	density = remap(density, 1.0 - globalCloudsCoverage, 1.0, 0.0, 1.0);
	// TODO: update this when a more efficient way for density calculation is implemented
	density *= globalCloudsDensity;

	// sample extra detail noise on the edges if isHighQuality
	if (isHighQuality) {
		// load detail shape texture (worley32 noise)
		vec3 detail = texture(worleyTex, cloudDetailScale * position).rgb;
		float detailFBM = dot(detail, cloudDetailWeights);

		// calculate detail density modifier
		float heightFraction = calculateCloudHeightFraction(position, cloud);
		float detailDensityModifier = mix(detailFBM, 1.0 - detailFBM, clamp(heightFraction * 10.0, 0.0, 1.0));

		// update the density with the calculated details
		density = remap(density, detailDensityModifier * cloudDetailMultiplier, 1.0, 0.0, 1.0);
	}

	// return clamped value
	return clamp(density, 0.0, 1.0);
}

// Calculates the color for the clouds
vec4 clouds(in ray view, in planet earth, in cloud cloud) 
{
	// prepare data for ray-cloud_layer intersections
	float distanceToCloudLow, distanceToCloudHigh, cloudLayer;
	// calculate ray-cloud_layer intersections
	rayCloudLayerIntersection(view, cloud, distanceToCloudLow, distanceToCloudHigh, cloudLayer);
	// calculate distance to cloud layer (for above, below and inside look of the clouds)
	float distanceToCloudLayer = min(distanceToCloudLow, distanceToCloudHigh);

	// initialize variables for ray-marching
	float distancePassed = 0.0f;
	vec3 color = vec3(0.0);
	float transmittance = 1.0f;

	// calculate the view ray segment length
	float segmentLength = cloudLayer / float(VIEW_RAY_SAMPLES);

	// move ray origin to the intersection with cloud lower layer
	view.origin += view.direction * distanceToCloudLayer;
	// update the distance passed
	distancePassed += distanceToCloudLayer;

	// iterate over view-ray direction
	for (int i = 0; i < VIEW_RAY_SAMPLES; ++i) {
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
			float density = calculateCloudDensity(samplePosition, true, cloud);
			// calculate transmittance
			transmittance *= exp(- density * segmentLength);
			// calculate the color if the density is above zero
			// TODO: cloudsColor * 0.5 should be updated with the light ray-march calculation
			if (density > 0.0) color += cloudsColor * 0.5 * density * segmentLength * transmittance;
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
	// recalculate space
	vec2 q = gl_FragCoord.xy / resolution.xy;
	vec2 p = -1.0 + 2.0*q;
	p.x *= resolution.x / resolution.y;
	
	// calculate ray direction
    vec3 uu = normalize(cross(cameraDirection, cameraUp));
    vec3 vv = normalize(cross(uu, cameraDirection));
	vec3 rd = normalize(p.x * uu + p.y * vv + cameraDirection);

	// translate camera for earth radius
	vec3 ro = cameraPosition + vec3(0.0, earthRadius, 0.0);
	
	// create view ray
	ray view = ray(ro, rd);
	
	// prepare earth info
	planet earth = planet(earthRadius, atmosphereRadius);
	
	// prepare cloud info
	cloud cloud = cloud(earthRadius + cloudHeightLOW, earthRadius + cloudHeightHIGH);

	// calculate the clouds color
	vec4 clouds = clouds(view, earth, cloud);

	// output the final result
	gl_FragColor = clouds;
}