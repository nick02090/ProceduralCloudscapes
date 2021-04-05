#include "SkyboxEnvironment.h"

SkyboxEnvironment::SkyboxEnvironment()
{
	// Initialize member variables
	type = EnvironmentType::Skybox;
	data = new SkyboxEnvironmentData();

	// Initialize skybox properties
	SkyboxEnvironmentData* skyboxData = static_cast<SkyboxEnvironmentData*>(data);
	skyboxData->sunDirection = glm::vec3(0.f, 1.f, 0.f);
	skyboxData->earthRadius = 6371e3;
	skyboxData->atmosphereRadius = 6420e3;
	skyboxData->Hr = 7994;
	skyboxData->Hm = 1200;
	skyboxData->betaR = glm::vec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
	skyboxData->betaM = glm::vec3(21e-6f);
}

SkyboxEnvironment::~SkyboxEnvironment()
{
}

void SkyboxEnvironment::update()
{
}
