#include "Skybox.h"

Skybox::Skybox(Window* _window) : SceneObject(_window)
{
	// Initialize member variables
	sunDirection = glm::vec3(0.f, 1.f, 0.f);
	earthRadius = 6371e3;
	atmosphereRadius = 6420e3;
	Hr = 7994;
	Hm = 1200;
	betaR = glm::vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
	betaM = glm::vec3(21e-6f);
}

Skybox::~Skybox()
{
}

void Skybox::update()
{
}
