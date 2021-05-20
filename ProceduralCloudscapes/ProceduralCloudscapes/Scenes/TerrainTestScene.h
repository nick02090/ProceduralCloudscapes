#ifndef TERRAIN_TEST_SCENE
#define TERRAIN_TEST_SCENE

#include "../Engine/Scene.h"

class Shader;

class TerrainTestScene : public Scene
{
public:
	TerrainTestScene(Window* _window);
	~TerrainTestScene();

	void update() override;
};


#endif // !TERRAIN_TEST_SCENE
