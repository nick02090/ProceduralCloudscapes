#include "PBRTestScene.h"
#include "../SceneObjects/Sphere.h"


PBRTestScene::PBRTestScene(Window* _window) : Scene(_window, "PBR Test Scene", EnvironmentType::Skybox)
{
	Sphere* grassSphere = new Sphere(_window, "Textures/grass/", glm::vec3(-2.5f, 10.f, 0.f));
	Sphere* rockSphere = new Sphere(_window, "Textures/rocks/", glm::vec3(0.f, 10.f, 0.f));
	Sphere* snowSphere = new Sphere(_window, "Textures/snow/", glm::vec3(2.5f, 10.f, 0.f));
	addSceneObject(grassSphere);
	addSceneObject(rockSphere);
	addSceneObject(snowSphere);
}

PBRTestScene::~PBRTestScene()
{
}

void PBRTestScene::update()
{
}
