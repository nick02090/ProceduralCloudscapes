#ifndef TERRAIN_H
#define TERRAIN_H

#include "../Engine/SceneObject.h"
#include "../Engine/Color.h"

class Shader;
class Texture;
class PBRMaterial;

struct TerrainNoise {
	float amplitude;
	float frequency;
	int octaves;
	float lacunarity;
	float gain;
	glm::vec2 seed;
	float power;

	float frequencyMultiplier;
};

struct TerrainData {
	// =============================================
	// TERRAIN SHAPE
	// =============================================
	
	// number of subdivisions used in base plane
	size_t subdivision;
	// scale of the base plane
	float scale;
	// terrain noise parameters
	TerrainNoise terrainNoise;
	// flag for showing terrain in wireframe mode
	bool wireframe;
	// size of one tile (base plane)
	size_t tileSize;

	// =============================================
	// TERRAIN COLOR
	// =============================================

	// amount ([0.0, 1.0]) of grass that will cover the bottom terrain
	float grassCoverage;
	// amount ([0.0, 1.0]) of snow that will cover the top terrain
	float snowCoverage;
	// base color that will define the grass shading
	Color grassColor;
	// base color that will define the rock shading
	Color rockColor;
	// base color that will define the snow shading
	Color snowColor;
	// scale of the grass textures loading
	float grassScale;
	// scale of the rock textures loading
	float rockScale;
	// scale of the snow textures loading
	float snowScale;

	// =============================================
	// POST PROCESSING
	// =============================================

	// amount of fog that will cover the terrain
	float fogFalloff;
	// color of the fog
	Color fogColor;
	// flag that determines whether the part of day will inflict color of the fog
	bool isRealFog;
};

class Terrain : public SceneObject, public GUIBuilder {
public:
	Terrain(Window* _window);
	~Terrain();

	void update() override;
	void buildGUI() override;


	// GETTERS

	inline size_t getSubdivisions() const { return data->subdivision; }
	inline size_t getResolution() const { return data->subdivision * 2; }
	inline float getScale() const { return data->scale; }
	inline bool getWireframe() const { return data->wireframe; }
	inline float getGrassCoverage() const { return data->grassCoverage; }
	inline float getSnowCoverage() const { return data->snowCoverage; }
	inline Color getGrassColor() const { return data->grassColor; }
	inline Color getRockColor() const { return data->rockColor; }
	inline Color getSnowColor() const { return data->snowColor; }
	inline float getGrassScale() const { return data->grassScale; }
	inline float getRockScale() const { return data->rockScale; }
	inline float getSnowScale() const { return data->snowScale; }
	inline float getFogFalloff() const { return data->fogFalloff; }
	inline Color getFogColor() const { return data->fogColor; }
	inline bool getIsRealFog() const { return data->isRealFog; }

	// SETTERS

	inline void setSubdivisions(size_t _subdivision) { data->subdivision = _subdivision; }
	inline void setScale(float _scale) { data->scale = _scale; }
	inline void setWireframe(bool _isWireframe) { data->wireframe = _isWireframe; }
	inline void setGrassCoverage(float _grassCoverage) { data->grassCoverage = _grassCoverage; }
	inline void setSnowCoverage(float _snowCoverage) { data->snowCoverage = _snowCoverage; }
	inline void setGrassColor(Color _grassColor) { data->grassColor = _grassColor; }
	inline void setRockColor(Color _rockColor) { data->rockColor = _rockColor; }
	inline void setSnowColor(Color _snowColor) { data->snowColor = _snowColor; }
	inline void setGrassScale(float _grassScale) { data->grassScale = _grassScale; }
	inline void setRockScale(float _rockScale) { data->rockScale = _rockScale; }
	inline void setSnowScale(float _snowScale) { data->snowScale = _snowScale; }
	inline void setFogFalloff(float _fogFalloff) { data->fogFalloff = _fogFalloff; }
	inline void setFogColor(Color _fogColor) { data->fogColor = _fogColor; }
	inline void setIsRealFog(bool _isRealFog) { data->isRealFog = _isRealFog; }

private:
	void generateTerrainData();
	void updatePositionData();
	glm::vec2 calculateCurrentCameraTile();

	unsigned int terrainVAO, terrainVBO, terrainEBO;
	unsigned int positionBuffer;

	std::vector<glm::vec2> positionBufferData;

	TerrainData* data = nullptr;

	Shader* shader = nullptr;

	PBRMaterial* grassMaterial = nullptr;
	PBRMaterial* rockMaterial = nullptr;
	PBRMaterial* snowMaterial = nullptr;
};

#endif // !TERRAIN_H
