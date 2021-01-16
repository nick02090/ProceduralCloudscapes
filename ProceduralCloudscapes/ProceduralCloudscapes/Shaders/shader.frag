#version 460 core
struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Shading {
	vec3 color;
	float strength;
	vec3 texel;
};

struct DirectionalLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;

	float cosInnerCutoff;
	float cosOuterCutoff;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 LightPos;
in vec2 TexCoords;

uniform Material material;
uniform DirectionalLight directionalLight;
uniform PointLight pointLight;
uniform SpotLight spotLight;

float calculateDiffuseStrength(vec3 normal, vec3 lightDirection)
{
	return max(dot(normal, lightDirection), 0.0);
}

float calculateSpecularStrength(vec3 lightDirection, vec3 normal, vec3 viewDirection, float shininess)
{
	vec3 reflectDirection = reflect(-lightDirection, normal);
	return pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);
}

vec3 calculateShading(Shading shading)
{
	return (shading.color * shading.strength * shading.texel);
}

vec3 calculateAmbientShading(vec3 color, vec3 texel)
{
	Shading shading = Shading(color, 1.0, texel);
	return calculateShading(shading);
}

vec3 calculateDiffuseShading(vec3 color, vec3 texel, vec3 normal, vec3 lightDirection)
{
	float strength = calculateDiffuseStrength(normal, lightDirection);
	Shading shading = Shading(color, strength, texel);
	return calculateShading(shading);
}

vec3 calculateSpecularShading(vec3 color, vec3 texel, vec3 normal, vec3 lightDirection, vec3 viewDirection, float shininess)
{
	float strength = calculateSpecularStrength(lightDirection, normal, viewDirection, shininess);
	Shading shading = Shading(color, strength, texel);
	return calculateShading(shading);
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
	vec3 lightDirection = normalize(-light.direction);
	// calculate texels
	vec3 diffuseTexel = vec3(texture(material.diffuse, TexCoords));
	vec3 specularTexel = vec3(texture(material.specular, TexCoords));
	// calculate shading
	vec3 ambient = calculateAmbientShading(light.ambient, diffuseTexel);
	vec3 diffuse = calculateDiffuseShading(light.diffuse, diffuseTexel, normal, lightDirection);
	vec3 specular = calculateSpecularShading(light.specular, specularTexel, normal, lightDirection, viewDirection, material.shininess);
	return (ambient + diffuse + specular);
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDirection)
{
	vec3 lightDirection = normalize(light.position - FragPos);
	// attenuation
	float pointDistance = length(light.position - FragPos);
	float pointAttenuation = 1.0 / (light.constant + light.linear * pointDistance +
									light.quadratic * (pointDistance * pointDistance));
	// calculate texels
	vec3 diffuseTexel = vec3(texture(material.diffuse, TexCoords));
	vec3 specularTexel = vec3(texture(material.specular, TexCoords));
	// calculate shading
	vec3 ambient = calculateAmbientShading(light.ambient, diffuseTexel);
	vec3 diffuse = calculateDiffuseShading(light.diffuse, diffuseTexel, normal, lightDirection);
	vec3 specular = calculateSpecularShading(light.specular, specularTexel, normal, lightDirection, viewDirection, material.shininess);
	ambient *= pointAttenuation;
	diffuse *= pointAttenuation;
	specular *= pointAttenuation;
	return (ambient + diffuse + specular);
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDirection)
{
	vec3 lightDirection = normalize(light.position - FragPos);
	// smooth edges
	float theta = dot(lightDirection, normalize(-light.direction));
	float epsilon = light.cosInnerCutoff - light.cosOuterCutoff;
	float intensity = clamp((theta - light.cosOuterCutoff) / epsilon, 0.0, 1.0);
	// calculate texels
	vec3 diffuseTexel = vec3(texture(material.diffuse, TexCoords));
	vec3 specularTexel = vec3(texture(material.specular, TexCoords));
	// calculate shading
	vec3 ambient = calculateAmbientShading(light.ambient, diffuseTexel);
	vec3 diffuse = calculateDiffuseShading(light.diffuse, diffuseTexel, normal, lightDirection);
	vec3 specular = calculateSpecularShading(light.specular, specularTexel, normal, lightDirection, viewDirection, material.shininess);
	diffuse *= intensity;
	specular *= intensity;
	return (ambient + diffuse + specular);
}

void main()
{
	// properties
	vec3 norm = normalize(Normal);
	vec3 viewDirection = normalize(-FragPos);

	// directional lighting
	vec3 result = calculateDirectionalLight(directionalLight, norm, viewDirection);
	result += calculatePointLight(pointLight, norm, viewDirection);
	result += calculateSpotLight(spotLight, norm, viewDirection);

	FragColor = vec4(result, 1.0);
}