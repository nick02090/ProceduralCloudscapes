#include "SkyboxTestScene.h"

#include "../SceneObjects/Clouds.h"

SkyboxTestScene::SkyboxTestScene(Window* _window) : Scene(_window, "Skybox Test Scene", EnvironmentType::Skybox)
{
	// Add clouds to the scene
	Clouds* clouds = new Clouds(window);
	addSceneObject(clouds);

	// Set higher movement speed for the camera
	window->getCamera()->setMovementSpeed(5000.f);
}

SkyboxTestScene::~SkyboxTestScene()
{
}

void SkyboxTestScene::update()
{
}
