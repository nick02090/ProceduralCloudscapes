#include "SkyboxTestScene.h"

#include "../Engine/Shader.h"
#include "../Engine/Environment/SkyboxEnvironment.h"
#include "../Engine/Utilities.h"

SkyboxTestScene::SkyboxTestScene(Window* _window) : Scene(_window, "Skybox Test Scene", EnvironmentType::Skybox)
{
}

SkyboxTestScene::~SkyboxTestScene()
{
}

void SkyboxTestScene::update()
{
}
