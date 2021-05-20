#ifndef TERRAIN_H
#define TERRAIN_H

#include "../Engine/SceneObject.h"

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

	// SETTERS

	inline void setSubdivisions(size_t _subdivision) { data->subdivision = _subdivision; }
	inline void setScale(float _scale) { data->scale = _scale; }
	inline void setWireframe(bool _isWireframe) { data->wireframe = _isWireframe; }

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
