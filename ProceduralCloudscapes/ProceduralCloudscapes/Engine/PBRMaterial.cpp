#include "PBRMaterial.h"

PBRMaterial::PBRMaterial(const char* texturesPath)
{
    // generate paths for the textures
    std::string baseTexturePath(texturesPath);
    // albedo
    std::string albedoPath(baseTexturePath);
    albedoPath.append("albedo.png");
    // normal
    std::string normalPath(baseTexturePath);
    normalPath.append("normal.png");
    // metallic
    std::string metallicPath(baseTexturePath);
    metallicPath.append("metallic.png");
    // roughness
    std::string roughnessPath(baseTexturePath);
    roughnessPath.append("roughness.png");
    // ao
    std::string aoPath(baseTexturePath);
    aoPath.append("ao.png");

    // load and create textures
    albedoTex = new Texture(albedoPath.c_str());
    normalTex = new Texture(normalPath.c_str());
    metallicTex = new Texture(metallicPath.c_str());
    roughnessTex = new Texture(roughnessPath.c_str());
    aoTex = new Texture(aoPath.c_str());
}

PBRMaterial::~PBRMaterial()
{
	delete albedoTex;
	delete normalTex;
	delete metallicTex;
	delete roughnessTex;
	delete aoTex;
}
