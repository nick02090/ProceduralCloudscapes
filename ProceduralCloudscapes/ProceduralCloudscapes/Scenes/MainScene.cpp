#include "MainScene.h"

#include "../SceneObjects/Clouds.h"
#include "../SceneObjects/Terrain.h"

MainScene::MainScene(Window* _window) : Scene(_window, "Iscrtavanje volumetrijskih oblaka u stvarnom vremenu", EnvironmentType::Skybox)
{
	// Add clouds to the scene
	Clouds* clouds = new Clouds(window);
	addSceneObject(clouds);

	// Add terrain to the scene
	Terrain* terrain = new Terrain(window);
	addSceneObject(terrain);

	// Set initial camera movement speed
	window->getCamera()->setMovementSpeed(5000.f);
}

MainScene::~MainScene()
{
}

void MainScene::update()
{
}
