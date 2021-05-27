#version 460 core
//===============================================================================================
// CONSTANTS 
//===============================================================================================

#define GRASS_COVERAGE_MULTIPLIER 0.3f
#define SNOW_COVERAGE_MULTIPLIER 0.4f
#define TERRAIN_SHININESS 32

// Sun
const float sunAngularDiameter = 0.009250245; // deg2rad(0.53)
const float earthRadius = 6360e3f;

// Math
const float PI = 3.14159265358979323846;
const float PI_2 = 1.57079632679489661923;
const float PI_4 = 0.785398163397448309616;

//===============================================================================================
// STRUCTS
//===============================================================================================


struct ray {
	vec3 position;
	vec3 direction;
};

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
	vec3 color;
};

struct material {
	vec3 albedo;
	vec3 normal;
	float metallic;
	float roughness;
	float ao;
	vec3 baseColor;
    float scale;
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

// Fog
uniform float fogFalloff = 0.f;
uniform vec3 fogColor  = vec3(0.5, 0.6, 0.7);
uniform bool isRealFog = true;

// Heights
uniform float grassHeight = 5000.f;
uniform float rockHeight = 7000.f;

// MATERIALS
// Grass
layout (binding = 0) uniform sampler2D grassAlbedo;
layout (binding = 1) uniform sampler2D grassNormal;
layout (binding = 2) uniform sampler2D grassMetallic;
layout (binding = 3) uniform sampler2D grassRoughness;
layout (binding = 4) uniform sampler2D grassAO;
uniform vec3 grassBaseColor;
uniform float grassScale;
// Rock
layout (binding = 5) uniform sampler2D rockAlbedo;
layout (binding = 6) uniform sampler2D rockNormal;
layout (binding = 7) uniform sampler2D rockMetallic;
layout (binding = 8) uniform sampler2D rockRoughness;
layout (binding = 9) uniform sampler2D rockAO;
uniform vec3 rockBaseColor;
uniform float rockScale;
// Snow
layout (binding = 10) uniform sampler2D snowAlbedo;
layout (binding = 11) uniform sampler2D snowNormal;
layout (binding = 12) uniform sampler2D snowMetallic;
layout (binding = 13) uniform sampler2D snowRoughness;
layout (binding = 14) uniform sampler2D snowAO;
uniform vec3 snowBaseColor;
uniform float snowScale;

// Sun
uniform float sunAltitude; // from range [0.0, 1.0] where 0.0 is night and 1.0 is clear day
uniform float sunAzimuth; // from range [-1.0, 1.0] where 0.0 is in front and (-)1.0 is behind
uniform float sunIntensity;
uniform vec3 sunColorDay = vec3(1.f, 0.96f, 0.9f);
uniform vec3 sunColorSunset = vec3(0.36f, 0.14f, 0.07f);

// Camera
uniform vec3 cameraPosition;
uniform vec2 resolution;
uniform mat4 inverseProjection;
uniform mat4 inverseView;

//===============================================================================================
// METHODS (PBR)
//===============================================================================================

vec3 getNormalFromMap(sampler2D normalMap, vec3 Normal, vec2 TexCoords, vec3 WorldPosition)
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(WorldPosition);
    vec3 Q2 = dFdy(WorldPosition);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}


float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}


float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

// Loads PBR textures
material loadPBRTextures(in sampler2D albedoMap, in sampler2D normalMap, in sampler2D metallicMap, in sampler2D roughnessMap, in sampler2D aoMap, 
                    in vec3 baseColor, in float scale, in vec2 TexCoords, in vec3 Normal, in vec3 WorldPosition) {
	vec3 albedo = texture(albedoMap, TexCoords * scale).rgb;
    vec3 normal = getNormalFromMap(normalMap, Normal, TexCoords, WorldPosition);
    float metallic = texture(metallicMap, TexCoords * scale).r;
    float roughness = texture(roughnessMap, TexCoords * scale).r;
    float ao = texture(aoMap, TexCoords * scale).r;
    return material(albedo, normal, metallic, roughness, ao, baseColor, scale);
}

// Calculates PBR color for given material
vec3 calculatePBR(material material, sun sun, ray sunRay, vec2 TexCoords, vec3 WorldPosition, vec3 Normal) {
    vec3 N = material.normal;
    vec3 V = normalize(cameraPosition - WorldPosition);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, material.albedo, material.metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    // calculate per-light radiance
    vec3 L = sunRay.direction;
    vec3 H = normalize(V + L);
    float distance = length(sunRay.position - WorldPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = sun.color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, material.roughness);
    float G = GeometrySmith(N, V, L, material.roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 nominator = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;
    
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - material.metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    Lo += (kD * material.albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    
    // calculate ambient lighting
    vec3 ambient = material.baseColor * material.albedo * material.ao;
    
    vec3 color = ambient + Lo;
    color *= sun.intensity / 20.f;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 
	
    return color;
}

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
vec3 calculateColor(vec3 position, vec3 normal, vec2 texCoords, sun sun, ray sunRay) {
	// Prepare resulting color
	vec3 color;
	// Calculate offset normals with interpolated noise (used as blending coefficients)
	float grassRockBlend = abs(dot(offsetNormal(position, 1.0f), vec3(0.0, 1.0, 0.0)));
	float snowRockBlend = abs(dot(offsetNormal(position, 2.0f), vec3(0.0, 1.0, 0.0)));
	// Calculate resulting color based on coverages and height
	if (grassRockBlend > (1.0 - grassCoverage * GRASS_COVERAGE_MULTIPLIER) && position.y < rockHeight) {
        material grassMaterial = loadPBRTextures(grassAlbedo, grassNormal, grassMetallic, grassRoughness, grassAO, grassBaseColor, grassScale, texCoords, normal, position);
		color = calculatePBR(grassMaterial, sun, sunRay, texCoords, position, normal);
    }
	else if (snowRockBlend < snowCoverage * SNOW_COVERAGE_MULTIPLIER && position.y > grassHeight) {
        material snowMaterial = loadPBRTextures(snowAlbedo, snowNormal, snowMetallic, snowRoughness, snowAO, snowBaseColor, snowScale, texCoords, normal, position);
		color = calculatePBR(snowMaterial, sun, sunRay, texCoords, position, normal);
    }
	else {
        material rockMaterial = loadPBRTextures(rockAlbedo, rockNormal, rockMetallic, rockRoughness, rockAO, rockBaseColor, rockScale, texCoords, normal, position);
		color = calculatePBR(rockMaterial, sun, sunRay, texCoords, position, normal);
    }
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

// Calculates amount of fog that will cover the terrain
float calculateFogAmount(vec3 cameraPosition, vec3 position, float fogFalloff) {
    float cameraToPointDistance = distance(cameraPosition, position);
    vec3 rayDirection = normalize(position - cameraPosition);
    float fogAmount = (1.0 - exp(-cameraToPointDistance * rayDirection.y * fogFalloff)) / rayDirection.y;
    return clamp(fogAmount, 0.0, 1.0);
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
	
	// Calculate the sun direction
	float cosSunAlt = cos(sunAlt);
	vec3 sunDirection = vec3(cos(sunAzi) * cosSunAlt, sin(sunAlt), sin(sunAzi) * cosSunAlt);

	// Calculate light color
	float sigmoid = 1 / (1.0 + exp(8.0 - sunDirection.y * 40.0));
	float a = min(max(sigmoid, 0.0f), 1.0f);
	float b = 1.0 - a;
	vec3 lightColor = sunColorDay * a + sunColorSunset * b;
	lightColor *= sunIntensity / 20.f;

	// Prepare sun info
    sun sun = sun(sunAlt, sunAzi, sunIntensity, sunAngularDiameter, sunColorDay, sunColorSunset, lightColor);
	ray sunRay = ray(vec3(0.f, 1000.f, 0.f), sunDirection);
	
	// Calculate base color
	vec3 color = calculateColor(WorldPos_FS_in, Normal_FS_in, TexCoord_FS_in, sun, sunRay);

	// Calculate color components
	vec3 ambient = ambient(lightColor);
	vec3 diffuse = diffuse(Normal_FS_in, sunDirection, lightColor);
	vec3 specular = specular(Normal_FS_in, sunDirection, rayDirection, lightColor);

    // Calculate final color
    color = color * (ambient + diffuse + specular);

    // ################################################
    //              POST - PROCESSING
    // ################################################

    // Calculate fog amount
    float fogAmount = calculateFogAmount(cameraPosition, WorldPos_FS_in, fogFalloff * 1e-6);

    // Calculate fog color
    sigmoid = 1 / (1.0 + exp(8.0 - WorldPos_FS_in.y * 40.0));
	a = min(max(sigmoid, 0.0f), 1.0f);
	b = 1.0 - a;
	vec3 fogColor = fogColor * 1.15f * a + fogColor * 0.85f * b;
    
    if (isRealFog)
        fogColor *= lightColor * 0.8;

    // Apply fog
    color = mix(color, fogColor, fogAmount);

	// Output final result
	gl_FragColor = vec4(color, 1.0);
}