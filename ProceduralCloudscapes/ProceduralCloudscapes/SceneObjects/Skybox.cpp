#include "Skybox.h"

#include "../Engine/Shader.h"
#include "../Engine/ScreenShader.h"
#include "../Engine/FrameBufferObject.h"

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

	// Build and compile shader programs
	shader = new Shader();
	shader->attachShader("Shaders/Default/shader.vert", ShaderInfo(ShaderType::kVertex));
	shader->attachShader("Shaders/Default/textureShader.frag", ShaderInfo(ShaderType::kFragment));
	shader->linkProgram();

	skyboxShader = new ScreenShader("Shaders/Skybox/skyboxShader.frag");

	// Configure other data
	configureData();
}

Skybox::~Skybox()
{
	delete shader;
	delete skyboxShader;
	delete framebuffer;
}

void Skybox::update()
{
}

void Skybox::configureData()
{
    // shader configuration
    shader->use();
    shader->setInt("texture1", 0);

    // framebuffer configuration
    framebuffer = new FrameBufferObject();
    // create a color attachment texture
    framebuffer->attachColorTexture((unsigned int)window->getWidth(), (unsigned int)window->getHeight());
    // create a depth attachment texture
    framebuffer->attachDepthTexture((unsigned int)window->getWidth(), (unsigned int)window->getHeight());
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (!framebuffer->checkStatus())
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
}
