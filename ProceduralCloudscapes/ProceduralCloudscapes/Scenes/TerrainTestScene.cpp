#include "TerrainTestScene.h"

#include "../SceneObjects/Terrain.h"
#include "../Engine/Environment/ColorEnvironment.h"

TerrainTestScene::TerrainTestScene(Window* _window) : Scene(_window, "Terrain Test Scene", EnvironmentType::Color)
{
	// Add terrain to the scene
	Terrain* terrain = new Terrain(window);
	addSceneObject(terrain);

	// Set higher movement speed for the camera
	window->getCamera()->setMovementSpeed(500.f);

	// setup environment color
	getEnvironment<ColorEnvironment>()->setColor(0.2f, 0.3f, 0.3f);
}

TerrainTestScene::~TerrainTestScene()
{
}

void TerrainTestScene::update()
{
}
