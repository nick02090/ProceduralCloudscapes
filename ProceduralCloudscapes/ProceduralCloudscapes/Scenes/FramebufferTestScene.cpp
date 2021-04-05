#include "FramebufferTestScene.h"
#include "../Engine/Shader.h"
#include "../Engine/ScreenShader.h"
#include "../Engine/Utilities.h"
#include "../Engine/FrameBufferObject.h"

#include <iostream>

FramebufferTestScene::FramebufferTestScene(Window* _window) : Scene(_window, "Framebuffer Test Scene", EnvironmentType::Color)
{
    // build and compile shader programs
    shader = new Shader();
    shader->attachShader("Shaders/Default/shader.vert", ShaderInfo(ShaderType::kVertex));
    shader->attachShader("Shaders/Default/textureShader.frag", ShaderInfo(ShaderType::kFragment));
    shader->linkProgram();

    screenShader = new ScreenShader("Shaders/FramebufferTest/screenShader.frag");

    configureData();
}

FramebufferTestScene::~FramebufferTestScene()
{
    delete shader;
    delete screenShader;
    delete framebuffer;
}

void FramebufferTestScene::update()
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
    // cubes
    glBindVertexArray(cubeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
    shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // floor
    glBindVertexArray(planeVAO);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    shader->setMat4("model", glm::mat4(1.0f));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    FrameBufferObject::unbind();
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader->getShader()->use();
    screenShader->draw(framebuffer->getColorTextureID(0));

    glm::vec3 camPos = camera->getPosition();
    std::cout << camPos.x << " " << camPos.y << " " << camPos.z << std::endl;
}

void FramebufferTestScene::configureData()
{
    // cube VAO
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    // plane VAO
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // load textures
    cubeTexture = util::loadTexture("Textures/container2.png");
    floorTexture = util::loadTexture("Textures/metal.png");

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
