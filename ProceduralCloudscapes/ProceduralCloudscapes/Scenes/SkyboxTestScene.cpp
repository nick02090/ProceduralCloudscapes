#include "SkyboxTestScene.h"

#include "../SceneObjects/Skybox.h"
#include "../Engine/Shader.h"

SkyboxTestScene::SkyboxTestScene(Window* _window) : Scene(_window, EnvironmentType::Color)
{
	// add skybox object to the scene
	Skybox* skybox = new Skybox(window);
	sceneObjects.push_back(skybox);
}

SkyboxTestScene::~SkyboxTestScene()
{
}

void SkyboxTestScene::update()
{
}
