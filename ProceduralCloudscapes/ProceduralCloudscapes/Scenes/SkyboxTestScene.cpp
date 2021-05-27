#include "SkyboxTestScene.h"

SkyboxTestScene::SkyboxTestScene(Window* _window) : Scene(_window, "Skybox Test Scene", EnvironmentType::Skybox)
{
	// Set higher movement speed for the camera
	window->getCamera()->setMovementSpeed(5000.f);
}

SkyboxTestScene::~SkyboxTestScene()
{
}

void SkyboxTestScene::update()
{
}
