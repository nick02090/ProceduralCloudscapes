#include "CloudsTestScene.h"

#include "../SceneObjects/Clouds.h"

CloudsTestScene::CloudsTestScene(Window* _window) : Scene(_window, "Clouds Test Scene", EnvironmentType::Skybox)
{
	// Add clouds to the scene
	Clouds* clouds = new Clouds(window);
	addSceneObject(clouds);

	window->getCamera()->setMovementSpeed(5000.f);
}

CloudsTestScene::~CloudsTestScene()
{
}

void CloudsTestScene::update()
{
}
