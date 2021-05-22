#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H

#include "Texture.h"

class PBRMaterial {
public:
	PBRMaterial(const char* texturesPath);
	~PBRMaterial();

	inline Texture* getAlbedo() const { return albedoTex; }
	inline Texture* getNormal() const { return normalTex; }
	inline Texture* getMetallic() const { return metallicTex; }
	inline Texture* getRoughness() const { return roughnessTex; }
	inline Texture* getAO() const { return aoTex; }

private:
	Texture* albedoTex = nullptr;
	Texture* normalTex = nullptr;
	Texture* metallicTex = nullptr;
	Texture* roughnessTex = nullptr;
	Texture* aoTex = nullptr;
};

#endif // !PBR_MATERIAL_H
