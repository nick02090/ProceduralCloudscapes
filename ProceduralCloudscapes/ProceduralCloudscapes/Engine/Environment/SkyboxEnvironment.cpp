#include "SkyboxEnvironment.h"

SkyboxEnvironment::SkyboxEnvironment()
{
	// Initialize member variables
	type = EnvironmentType::Skybox;
	data = new SkyboxData();
}

SkyboxEnvironment::~SkyboxEnvironment()
{
}

void SkyboxEnvironment::update()
{
}
