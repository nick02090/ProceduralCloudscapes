#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// Sun
uniform float sunAltitude; // from range [0.0, 1.0] where 0.0 is night and 1.0 is clear day
uniform float sunAzimuth; // from range [-1.0, 1.0] where 0.0 is in front and (-)1.0 is behind
uniform float sunIntensity;
uniform vec3 sunColorDay = vec3(1.0);
uniform vec3 sunColorSunset = vec3(0.5, 0.3, 0.1);
const float earthRadius = 6360e3f;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform vec3 baseColor = vec3(1.);

uniform vec3 camPos;

const float PI = 3.14159265358979323846;
const float PI_2 = 1.57079632679489661923;
const float PI_4 = 0.785398163397448309616;

const float sunAngularDiameter = 0.009250245; // deg2rad(0.53)

struct sun {
	float altitude;
	float azimuth;
	float intensity;
	float angularDiameter;
	vec3 colorDay;
	vec3 colorSunset;
};

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{	
	// calculate sun altitude and azimuth
	float sunAlt = 4.0 * - sunAngularDiameter + 1.6 * PI_4 * (0.5 + cos((1.0 - sunAltitude) * 3.0) / 2.0);
	float sunAzi = (1.0 - sunAzimuth * 0.7) * 4.6;

	// prepare sun info
    sun sun = sun(sunAlt, sunAzi, sunIntensity, sunAngularDiameter, sunColorDay, sunColorSunset);
    
	// calculate the sun direction
	float cosSunAlt = cos(sun.altitude);
	vec3 sunDirection = vec3(cos(sun.azimuth) * cosSunAlt, sin(sun.altitude), sin(sun.azimuth) * cosSunAlt);
    
	// Calculate light color
	float sigmoid = 1 / (1.0 + exp(8.0 - sunDirection.y * 40.0));
	float a = min(max(sigmoid, 0.0f), 1.0f);
	float b = 1.0 - a;
	vec3 lightColor = sun.colorDay * a + sun.colorSunset * b;
	lightColor *= sun.intensity / 20.f;

    vec3 lightPosition = vec3(0.0, 10.f, 0.0);

    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    float metallic  = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao        = texture(aoMap, TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    // calculate per-light radiance
    vec3 L = sunDirection;
    vec3 H = normalize(V + L);
    float distance = length(lightPosition - WorldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
    vec3 nominator    = NDF * G * F; 
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
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = baseColor * albedo * ao;
    
    vec3 color = ambient + Lo;
    color *= sun.intensity / 20.f;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}