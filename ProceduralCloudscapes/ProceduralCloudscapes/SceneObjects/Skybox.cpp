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

	// Build and compile shader program
	skyboxShader = new ScreenShader("Shaders/RaymarchTest/screenShader.frag");

	// Configure other data
	configureData();
}

Skybox::~Skybox()
{
	delete skyboxShader;
	delete framebuffer;
}

void Skybox::update()
{
    Camera* camera = window->getCamera();

    // bind to framebuffer and draw scene as we normally would to color texture 
    framebuffer->bind();
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

    // make sure we clear the framebuffer's content
    framebuffer->clear();

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    FrameBufferObject::unbind();
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);

    Shader* ssShader = skyboxShader->getShader();
    ssShader->use();
    ssShader->setVec3("cameraPos", camera->getPosition());
    ssShader->setVec3("lookAt", camera->getDirection());
    ssShader->setFloat("zoom", camera->getZoom());
    skyboxShader->draw(framebuffer->getColorTextureID(0));
}

void Skybox::configureData()
{
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
