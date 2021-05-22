#ifndef TERRAIN_H
#define TERRAIN_H

#include "../Engine/SceneObject.h"
#include "../Engine/Color.h"

class Shader;

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

	float grassCoverage;
	float snowCoverage;
	Color grassColor;
	Color rockColor;
	Color snowColor;
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

	// SETTERS

	inline void setSubdivisions(size_t _subdivision) { data->subdivision = _subdivision; }
	inline void setScale(float _scale) { data->scale = _scale; }
	inline void setWireframe(bool _isWireframe) { data->wireframe = _isWireframe; }
	inline void setGrassCoverage(float _grassCoverage) { data->grassCoverage = _grassCoverage; }
	inline void setSnowCoverage(float _snowCoverage) { data->snowCoverage = _snowCoverage; }
	inline void setGrassColor(Color _grassColor) { data->grassColor = _grassColor; }
	inline void setRockColor(Color _rockColor) { data->rockColor = _rockColor; }
	inline void setSnowColor(Color _snowColor) { data->snowColor = _snowColor; }

private:
	void generateTerrainData();
	void updatePositionData();

	unsigned int terrainVAO, terrainVBO, terrainEBO;
	unsigned int positionBuffer;

	std::vector<glm::vec2> positionBufferData;

	TerrainData* data = nullptr;

	Shader* shader = nullptr;
};

#endif // !TERRAIN_H
