#include "SkyboxEnvironment.h"
#include "../ScreenShader.h"
#include "../FrameBufferObject.h"
#include "../Utilities.h"

SkyboxEnvironment::SkyboxEnvironment(Window* _window) : Environment(_window)
{
	// Initialize member variables
	type = EnvironmentType::Skybox;
	data = new SkyboxEnvironmentData();

	// Initialize skybox properties
	SkyboxEnvironmentData* skyboxData = static_cast<SkyboxEnvironmentData*>(data);
	skyboxData->sunAltitude = 1.0f;
	skyboxData->sunAzimuth = 0.0f;
	skyboxData->sunIntensity = 20.0f;

	// Build and compile shader program
	skyboxShader = new ScreenShader("Shaders/Skybox/sky.frag");
}

SkyboxEnvironment::~SkyboxEnvironment()
{
	delete skyboxShader;
}

void SkyboxEnvironment::update()
{
	Camera* camera = window->getCamera();

	// configure shader data
	Shader* shader = skyboxShader->getShader();
	shader->use();

	// set shaders camera info
	shader->setMat4("inverseProjection", glm::inverse(window->getProjectionMatrix()));
	shader->setMat4("inverseView", glm::inverse(camera->getViewMatrix()));
	shader->setVec2("resolution", window->getSize());

	// set shaders sky info
	shader->setFloat("sunAltitude", getSunAltitude());
	shader->setFloat("sunAzimuth", getSunAzimuth());
	shader->setFloat("sunIntensity", getSunIntensity());
}

void SkyboxEnvironment::extendGUI()
{
	// Sun altitude
	float sunAltitude = getSunAltitude();
	ImGui::SliderFloat("Sun altitude", &sunAltitude, 0.0f, 1.0f);
	setSunAltitude(sunAltitude);

	// Sun azimuth
	float sunAzimuth = getSunAzimuth();
	ImGui::SliderFloat("Sun azimuth", &sunAzimuth, -1.0f, 1.0f);
	setSunAzimuth(sunAzimuth);

	// Sun intensity
	float sunIntensity = getSunIntensity();
	ImGui::SliderFloat("Sun intensity", &sunIntensity, 10.0f, 30.0f);
	setSunIntensity(sunIntensity);
}
