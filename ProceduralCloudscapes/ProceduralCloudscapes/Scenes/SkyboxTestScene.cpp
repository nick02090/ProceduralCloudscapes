#include "SkyboxTestScene.h"

#include "../Engine/Shader.h"
#include "../Engine/Environment/SkyboxEnvironment.h"
#include "../Engine/Utilities.h"
#include "../SceneObjects/Clouds.h"

SkyboxTestScene::SkyboxTestScene(Window* _window) : Scene(_window, "Skybox Test Scene", EnvironmentType::Skybox)
{
	Clouds* clouds = new Clouds(window);
	sceneObjects.push_back(clouds);
}

SkyboxTestScene::~SkyboxTestScene()
{
}

void SkyboxTestScene::update()
{
}
