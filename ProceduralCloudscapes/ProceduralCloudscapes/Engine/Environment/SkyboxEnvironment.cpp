#include "SkyboxEnvironment.h"
#include "../ScreenShader.h"
#include "../FrameBufferObject.h"

SkyboxEnvironment::SkyboxEnvironment(Window* _window) : Environment(_window)
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

	// Build and compile shader program
	skyboxShader = new ScreenShader("Shaders/RaymarchTest/screenShader.frag");
}

SkyboxEnvironment::~SkyboxEnvironment()
{
	delete skyboxShader;
}

void SkyboxEnvironment::update()
{
	Camera* camera = window->getCamera();

	// disable depth test while drawing the sky (so the sky is in the background of everything)
	glDisable(GL_DEPTH_TEST);

	// configure shader data
	Shader* ssShader = skyboxShader->getShader();
	ssShader->use();
	ssShader->setVec3("cameraPos", camera->getPosition());
	ssShader->setVec3("lookAt", camera->getDirection());
	ssShader->setFloat("zoom", camera->getZoom());

	// draw the sky
	skyboxShader->draw(0);

	// enable the depth test back
	glEnable(GL_DEPTH_TEST);
}
