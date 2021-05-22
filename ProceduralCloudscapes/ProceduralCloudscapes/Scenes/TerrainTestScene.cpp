#include "TerrainTestScene.h"

#include "../SceneObjects/Terrain.h"

TerrainTestScene::TerrainTestScene(Window* _window) : Scene(_window, "Terrain Test Scene", EnvironmentType::Skybox)
{
	// Add terrain to the scene
	Terrain* terrain = new Terrain(window);
	addSceneObject(terrain);

	// Set higher movement speed for the camera
	window->getCamera()->setMovementSpeed(5000.f);
}

TerrainTestScene::~TerrainTestScene()
{
}

void TerrainTestScene::update()
{
}
