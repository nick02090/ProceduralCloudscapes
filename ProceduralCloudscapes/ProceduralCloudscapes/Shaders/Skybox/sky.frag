#version 460 core
//===============================================================================================
// INPUT 
//===============================================================================================

// Camera
uniform mat4 inverseProjection;
uniform mat4 inverseView;
uniform vec2 resolution;

// Sun
uniform float sunAltitude; // from range [0.0, 1.0] where 0.0 is night and 1.0 is clear day
uniform float sunAzimuth; // from range [-1.0, 1.0] where 0.0 is in front and (-)1.0 is behind
uniform float sunIntensity;
uniform vec3 sunColor;

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
const vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f); // Rayleigh scattering coefficient
const vec3 betaM = vec3(21e-6f); // Mie scattering coefficient
const float Hr = 7994; // Rayleigh scale height
const float Hm = 1200; // Mie scale heights
const float g = 0.76f; // Mie mean cosine
const int VIEW_RAY_SAMPLES = 12;
const int SUN_RAY_SAMPLES = 6;

//===============================================================================================
// STRUCTS
//===============================================================================================

struct scatteringInfo {
    vec3 coefficient;
    float scaleHeight;
};

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

//===============================================================================================
// METHODS
//===============================================================================================

// Calculates clip space coordinate (NDC space)
vec3 computeClipSpaceCoord(ivec2 fragCoord){
	vec2 rayNDC = 2.0*vec2(fragCoord.xy)/resolution.xy - 1.0;
	return vec3(rayNDC, 1.0);
}

// Checks and solves quadratic equation (f(x) = ax^2 + bx + c)
bool solveQuadratic(float a, float b, float c, out float x0, out float x1) {
    // calculate the discriminant of the function (b^2 - 4ac)
    float discriminant = b * b - 4.0 * a * c;
    // if discriminant is less than zero there is no solution
    if (discriminant < 0.0) return false;
    // pre-calculate the square root of discriminant and 2a
    float discriminantROOT = sqrt(discriminant);
    float a2 = 2.0 * a;
    // calculate the final solution
    x0 = (-b - discriminantROOT)/(a2);
    x1 = (-b + discriminantROOT)/(a2);
    return true;
}

// Calculate ray-sphere intersection with the assumption that the sphere is centered at the origin
bool raySphereIntersection(ray ray, float sphereRadius, out float t0, out float t1) {
    // create quadratic equation arguments (f(x) = ax^2 + bx + c)
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(ray.origin, ray.direction);
    float c = dot(ray.origin, ray.origin) - sphereRadius * sphereRadius;
    // check if equation has a solution
    return solveQuadratic(a, b, c, t0, t1);
}

// Checks and calculates if and where the ray intersects with the planets atmosphere
bool rayPlanetAtmosphereIntersection(in ray ray, in planet planet, out float tr)
{
	// prepare result data for ray-atmosphere intersection 
	float ta0, ta1;
    if (!raySphereIntersection(ray, planet.aRadius, ta0, ta1)) return false;
	// prepare result data for ray-planet intersection
	float tp0, tp1;
	raySphereIntersection(ray, planet.radius, tp0, tp1);
	// calculate final result
    tr = min(ta1, tp0) - ta0;
    return true;
}

// Calculates RAYLEIGH phase function value
float rayleighPhase(float mu) {
    return 3.0 / (16.0 * PI) * (1.0 + mu * mu);
}

// Calculates MIE phase function value
float miePhase(float mu, float g) {
	float g2 = g * g;
	float mu2 = mu * mu;
    return 3.0 / (8.0 * PI) * ((1.0 - g2) * (1.0 + mu2) / ((2.0 + g2) * pow(1.0 + g2 - 2.0 * g * mu, 1.5)));
}

// Calculates the color of the sky
vec3 sky(ray view, planet earth, sun sun, scatteringInfo rayleigh, scatteringInfo mie, float g)
{
	// prepare data for view-atmosphere intersection
    float t;
	// check view and atmosphere intersection
    if (!rayPlanetAtmosphereIntersection(view, earth, t) || t < 0.0) return vec3(0.0);
    // calculate the view ray segment length
    float segmentLength = t / float(VIEW_RAY_SAMPLES);

	// initialize variables for calculation
    float tCurrent = 0.0;
	vec3 rayleighSUM = vec3(0.0);
	vec3 mieSUM = vec3(0.0);
    float rayleighOpticalDepth = 0.0;
	float mieOpticalDepth = 0.0;

	// calculate the sun direction
	float cosSunAlt = cos(sun.altitude);
	vec3 sunDirection = vec3(cos(sun.azimuth) * cosSunAlt, sin(sun.altitude), sin(sun.azimuth) * cosSunAlt);

	// calculate the cosine of angle between the sun direction and the ray direction
	float mu = dot(view.direction, sunDirection);
	// calculate phase function values
	float rayleighPHASE = rayleighPhase(mu);
	float miePHASE = miePhase(mu, g);
    
	// iterate over view ray direction
    for (int i = 0; i < VIEW_RAY_SAMPLES; ++i)
	{
		// calculate current sample position (middle point) and height
		vec3 samplePosition = view.origin + (tCurrent + segmentLength * 0.5) * view.direction;
        float height = length(samplePosition) - earth.radius;
		// calculate optical depth for this height
        float hr = exp(-height / rayleigh.scaleHeight) * segmentLength;
        float hm = exp(-height / mie.scaleHeight) * segmentLength;
		// accumulate optical depth
		rayleighOpticalDepth += hr;
        mieOpticalDepth += hm;

		// initialize variables for calculation
		float rayleighOpticalDepthSun = 0.0;
        float mieOpticalDepthSun = 0.0;

		// prepare data for sun-atmosphere intersection
		float tSun;
		// check sun and atmosphere intersection
		if (rayPlanetAtmosphereIntersection(ray(samplePosition, sunDirection), earth, tSun))
		{
			// initialize variables for calculation
			float tCurrentSun = 0.0;
			// calculate the sun ray segment length
			float segmentLengthSun = tSun / float(SUN_RAY_SAMPLES);
			// iterate over sun ray direction
			for (int j = 0; j < SUN_RAY_SAMPLES; ++j)
			{
				// calculate current sample position (middle point) and height
				vec3 samplePositionSun = samplePosition + (tCurrentSun + segmentLengthSun * 0.5) * sunDirection;
				float heightSun = length(samplePositionSun) - earth.radius;
				// calculate optical depth for this height
				float hrSun = exp(-heightSun / rayleigh.scaleHeight) * segmentLengthSun;
				float hmSun = exp(-heightSun / mie.scaleHeight) * segmentLengthSun;
				// accumulate optical depth
				rayleighOpticalDepthSun += hrSun;
				mieOpticalDepthSun += hmSun;
				// increase current point
				tCurrentSun += segmentLengthSun;
			}
		}

		// calculate transmittance
		vec3 rayleighTransmittance = rayleigh.coefficient * (rayleighOpticalDepth + rayleighOpticalDepthSun);
        vec3 mieTransmittance = mie.coefficient * (mieOpticalDepth + mieOpticalDepthSun);
		// calculate attenuation
		vec3 attenuation = exp(-(rayleighTransmittance + mieTransmittance));

		// accumulate scattering
		rayleighSUM += hr * attenuation;
		mieSUM += hm * attenuation;

		// increase current point
        tCurrent += segmentLength;
    }

	// calculate RAYLEIGH and MIE light
	vec3 rayleighLIGHT = rayleighSUM * rayleigh.coefficient * rayleighPHASE;
	vec3 mieLIGHT = mieSUM * mie.coefficient * miePHASE;
	// calculate final sky color
    return sun.intensity * (rayleighLIGHT + mieLIGHT);
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

	// DONT translate camera for earth radius
	vec3 ro = vec3(0.0, earthRadius, 0.0);

	// calculate sun altitude and azimuth
	float sunAlt = 4.0 * - sunAngularDiameter + 1.6 * PI_4 * (0.5 + cos((1.0 - sunAltitude) * 3.0) / 2.0);
	float sunAzi = (1.0 - sunAzimuth * 0.7) * 4.6;

	// prepare sun info
    sun sun = sun(sunAlt, sunAzi, sunIntensity, sunAngularDiameter, sunColor);

	// create view ray
	ray view = ray(ro, rd);

	// prepare earth info
	planet earth = planet(earthRadius, atmosphereRadius);

	// prepare scattering info for sky calculation
	scatteringInfo rayleigh = scatteringInfo(betaR, Hr);
	scatteringInfo mie = scatteringInfo(betaM, Hm);

	// calculate the sky color
	vec3 result = sky(view, earth, sun, rayleigh, mie, g);
	
	// output the final result
	gl_FragColor = vec4(result, 1.0);
}