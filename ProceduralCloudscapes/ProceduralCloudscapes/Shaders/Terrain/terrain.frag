#version 460 core
//===============================================================================================
// CONSTANTS 
//===============================================================================================

#define GRASS_COVERAGE_MULTIPLIER 0.1f
#define SNOW_COVERAGE_MULTIPLIER 0.5f
#define TERRAIN_SHININESS 32

// Sun
const float sunAngularDiameter = 0.009250245; // deg2rad(0.53)

// Math
const float PI = 3.14159265358979323846;
const float PI_2 = 1.57079632679489661923;
const float PI_4 = 0.785398163397448309616;

//===============================================================================================
// STRUCTS
//===============================================================================================

struct fbm {
	float amplitude;
	float frequency;
	int octaves;
	float lacunarity;
	float gain;
	vec2 seed;
	float power;
};

struct sun {
	float altitude;
	float azimuth;
	float intensity;
	float angularDiameter;
	vec3 colorDay;
	vec3 colorSunset;
};

//===============================================================================================
// INPUT 
//===============================================================================================

// INPUT
in vec3 WorldPos_FS_in;
in vec3 Normal_FS_in;
in vec2 TexCoord_FS_in;

// Terrain noise parameters
uniform fbm terrainNoise;

// Terrain coverages
uniform float grassCoverage = 0.1;
uniform float snowCoverage = 1.0;

// Heights
uniform float grassHeight = 2000.f;
uniform float rockHeight = 9000.f;
uniform float snowHeight = 11000.f;

// Colors
uniform vec3 grassColor = vec3(0.0, 0.8, 0.1);
uniform vec3 rockColor = vec3(0.5, 0.5, 0.5);
uniform vec3 snowColor = vec3(0.9);

// Sun
uniform float sunAltitude; // from range [0.0, 1.0] where 0.0 is night and 1.0 is clear day
uniform float sunAzimuth; // from range [-1.0, 1.0] where 0.0 is in front and (-)1.0 is behind
uniform float sunIntensity;
uniform vec3 sunColorDay = vec3(1.f, 0.96f, 0.9f);
uniform vec3 sunColorSunset = vec3(0.36f, 0.14f, 0.07f);

// Camera
uniform vec2 resolution;
uniform mat4 inverseProjection;
uniform mat4 inverseView;

//===============================================================================================
// METHODS
//===============================================================================================

// Calculates clip space coordinate (NDC space)
vec3 computeClipSpaceCoord(ivec2 fragCoord){
	vec2 rayNDC = 2.0*vec2(fragCoord.xy)/resolution.xy - 1.0;
	return vec3(rayNDC, 1.0);
}

// Calculates a random value based on 2D vector (co) and a seed
float rand(vec2 co, vec2 seed){
    return fract(sin(dot(co, vec2(12.9898, 78.233) + seed)) * 43758.5453);
}

// Calculates interpolated noise
float interpolatedNoise(vec2 x, vec2 seed) {
    // Grid
    vec2 p = floor(x);
    vec2 w = fract(x);
	
    // Quintic interpolant
    vec2 u = w * w * w * (w * (w * 6.0 - 15.0) + 10.0);

	// Gradients
	float ga = rand(p + vec2(0.0, 0.0), seed);
	float gb = rand(p + vec2(1.0, 0.0), seed);
	float gc = rand(p + vec2(0.0, 1.0), seed);
	float gd = rand(p + vec2(1.0, 1.0), seed);

	// Interpolation
	return ga + 
			u.x * (gb - ga) + 
			u.y * (gc - ga) + 
			u.x * u.y * (gd - gc - gb + ga);
}

// Calculates fBm for the noise defined with fbm at coord
float noiseFBM(vec2 coord, fbm fbm) {
    // Initial values
    float frequency = fbm.frequency;
    float amplitude = fbm.amplitude;
    float noise = 0.0f;
    // Loop through octaves
    for (int i = 0; i < fbm.octaves; ++i) {
        // Initial value for this noise
        float noiseRes = interpolatedNoise(coord * frequency, fbm.seed);
        // Accumulate noise
        noise += amplitude * noiseRes;
        // Update properties
        frequency *= fbm.lacunarity;
        amplitude *= fbm.gain;
    }
    // Return final fBm noise
    return pow(noise, fbm.power);
}

// Calculates new normal for the position with offset for the interpolated noise
vec3 offsetNormal(vec3 position, float offset) {
	float dx = (noiseFBM(vec2((position.x + offset), position.z), terrainNoise) - noiseFBM(vec2((position.x - offset), position.z), terrainNoise))/(2.0*offset);
	float dz = (noiseFBM(vec2(position.x, (position.z + offset)), terrainNoise) - noiseFBM(vec2(position.x, (position.z - offset)), terrainNoise))/(2.0*offset);
	vec3 X = vec3(1.0, dx, 0.0);
	vec3 Z = vec3(0.0, dz, 1.0);
	// Calculate and normalize new normal
	return normalize(cross(Z,X));
}

// Calculates base color value based on the current position
vec3 calculateColor(vec3 position) {
	// Prepare resulting color
	vec3 color;
	// Calculate offset normals with interpolated noise (used as blending coefficients)
	float grassRockBlend = abs(dot(offsetNormal(position, 1.0f), vec3(0.0, 1.0, 0.0)));
	float snowRockBlend = abs(dot(offsetNormal(position, 2.0f), vec3(0.0, 1.0, 0.0)));
	// Calculate resulting color based on coverages and height
	if (grassRockBlend > (1.0 - grassCoverage * GRASS_COVERAGE_MULTIPLIER) && position.y < rockHeight) 
		color = grassColor;
	else if (snowRockBlend < snowCoverage * SNOW_COVERAGE_MULTIPLIER && position.y > grassHeight)
		color = snowColor;
	else
		color = rockColor;
	// Return final result
	return color;
}

// Calculates ambient component for the terrain
vec3 ambient(vec3 lightColor) {
	float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
    return ambient;
}

// Calculates diffuse component for the terrain
vec3 diffuse(vec3 normal, vec3 lightDirection, vec3 lightColor) {
	float diffuseStrength = max(dot(normal, lightDirection), 0.0);
	return diffuseStrength * lightColor;
}

// Calculates specular component for the terrain
vec3 specular(vec3 normal, vec3 lightDirection, vec3 viewDirection, vec3 lightColor){
	vec3 reflectDirection = reflect(-lightDirection, normal);
	float specularStrength = pow(max(dot(viewDirection, reflectDirection), 0.0), TERRAIN_SHININESS);
	return specularStrength * lightColor;
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
	vec3 rayDirection = normalize((inverseView * viewRay).xyz);

	// Calculate sun altitude and azimuth
	float sunAlt = 4.0 * - sunAngularDiameter + 1.6 * PI_4 * (0.5 + cos((1.0 - sunAltitude) * 3.0) / 2.0);
	float sunAzi = (1.0 - sunAzimuth * 0.7) * 4.6;

	// Prepare sun info
    sun sun = sun(sunAlt, sunAzi, sunIntensity, sunAngularDiameter, sunColorDay, sunColorSunset);
	
	// Calculate the sun direction
	float cosSunAlt = cos(sun.altitude);
	vec3 sunDirection = vec3(cos(sun.azimuth) * cosSunAlt, sin(sun.altitude), sin(sun.azimuth) * cosSunAlt);

	// Calculate light color
	float sigmoid = 1 / (1.0 + exp(8.0 - sunDirection.y * 40.0));
	float a = min(max(sigmoid, 0.0f), 1.0f);
	float b = 1.0 - a;
	vec3 lightColor = sun.colorDay * a + sun.colorSunset * b;
	lightColor *= sun.intensity / 20.f;
	
	// Calculate base color
	vec3 color = calculateColor(WorldPos_FS_in);

	// Calculate color components
	vec3 ambient = ambient(lightColor);
	vec3 diffuse = diffuse(Normal_FS_in, sunDirection, lightColor);
	vec3 specular = specular(Normal_FS_in, sunDirection, rayDirection, lightColor);

	// Output final result
	gl_FragColor = vec4(color * (ambient + diffuse + specular), 1.0);
}