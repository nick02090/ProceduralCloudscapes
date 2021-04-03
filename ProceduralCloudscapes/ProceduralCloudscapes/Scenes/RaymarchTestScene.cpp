#include "RaymarchTestScene.h"
#include "../Engine/Shader.h"
#include "../Engine/ScreenShader.h"
#include "../Engine/FrameBufferObject.h"

RaymarchTestScene::RaymarchTestScene(Window* _window) : Scene(_window, EnvironmentType::Color)
{
    // build and compile shader programs
    shader = new Shader();
    shader->attachShader("Shaders/Default/shader.vert", ShaderInfo(ShaderType::kVertex));
    shader->attachShader("Shaders/Default/textureShader.frag", ShaderInfo(ShaderType::kFragment));
    shader->linkProgram();

    screenShader = new ScreenShader("Shaders/RaymarchTest/screenShader.frag");

    configureData();
}

RaymarchTestScene::~RaymarchTestScene()
{
    delete shader;
    delete screenShader;
    delete framebuffer;
}

void RaymarchTestScene::update()
{
    Camera* camera = window->getCamera();

    // render
    // ------
    // bind to framebuffer and draw scene as we normally would to color texture 
    framebuffer->bind();
    glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

    // make sure we clear the framebuffer's content
    framebuffer->clear();

    shader->use();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera->getZoom()), (float)window->getWidth() / (float)window->getHeight(), 0.1f, 100.0f);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    FrameBufferObject::unbind();
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);

    Shader* ssShader = screenShader->getShader();
    ssShader->use();
    ssShader->setVec3("cameraPos", camera->getPosition());
    ssShader->setVec3("lookAt", camera->getDirection());
    ssShader->setFloat("zoom", camera->getZoom());
    screenShader->draw(framebuffer->getColorTextureID(0));
}

void RaymarchTestScene::configureData()
{
    // shader configuration
    shader->use();
    shader->setInt("texture1", 0);

    Shader* ssShader = screenShader->getShader();
    ssShader->use();
    ssShader->setInt("screenTexture", 0);

    // framebuffer configuration
    framebuffer = new FrameBufferObject();
    // create a color attachment texture
    framebuffer->attachColorTexture((unsigned int)window->getWidth(), (unsigned int)window->getHeight());
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    framebuffer->attachDepthTexture((unsigned int)window->getWidth(), (unsigned int)window->getHeight());
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (!framebuffer->checkStatus())
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
}
