#include "ShaderTestScene.h"

#include "../Engine/Shader.h"
#include "../Engine/Utilities.h"

ShaderTestScene::ShaderTestScene(Window* _window) : Scene(_window, EnvironmentType::Color)
{
    // build and compile shader programs
    shader = new Shader();
    shader->attachShader("Shaders/ShaderTest/shader.vert", ShaderInfo(ShaderType::kVertex));
    shader->attachShader("Shaders/ShaderTest/shader.frag", ShaderInfo(ShaderType::kFragment));
    shader->linkProgram();

    lightShader = new Shader();
    lightShader->attachShader("Shaders/ShaderTest/shader.vert", ShaderInfo(ShaderType::kVertex));
    lightShader->attachShader("Shaders/ShaderTest/lightShader.frag", ShaderInfo(ShaderType::kFragment));
    lightShader->linkProgram();

    // load and create a texture
    diffuseTex = util::loadTexture("Textures/container2.png");
    specularTex = util::loadTexture("Textures/container2_specular.png");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    configureData();

    // setup light position
    lightDir = glm::vec3(-0.2f, -1.0f, -0.3f);
    pointLigthPos = glm::vec3(1.2f, 1.0f, 2.0f);
}

ShaderTestScene::~ShaderTestScene()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);

	delete shader;
	delete lightShader;
}

void ShaderTestScene::update()
{
    Camera* camera = window->getCamera();

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularTex);

    // activate shader
    shader->use();
    shader->setInt("material.diffuse", 0);
    shader->setInt("material.specular", 1);
    shader->setFloat("material.shininess", 31);
    shader->setVec3("directionalLight.ambient", glm::vec3(0.2f));
    shader->setVec3("directionalLight.diffuse", glm::vec3(0.5f));
    shader->setVec3("directionalLight.specular", glm::vec3(1.0f));
    shader->setVec3("directionalLight.direction", lightDir);
          
    shader->setVec3("pointLight.ambient", glm::vec3(0.2f));
    shader->setVec3("pointLight.diffuse", glm::vec3(0.5f));
    shader->setVec3("pointLight.specular", glm::vec3(1.0f));
    shader->setFloat("pointLight.constant", 1.0f);
    shader->setFloat("pointLight.linear", 0.09f);
    shader->setFloat("pointLight.quadratic", 0.032f);
    shader->setVec3("pointLight.position", pointLigthPos);
          
    shader->setVec3("spotLight.ambient", glm::vec3(0.2f));
    shader->setVec3("spotLight.diffuse", glm::vec3(0.5f));
    shader->setVec3("spotLight.specular", glm::vec3(1.0f));
    shader->setFloat("spotLight.cosInnerCutoff", glm::cos(glm::radians(12.5f)));
    shader->setFloat("spotLight.cosOuterCutoff", glm::cos(glm::radians(17.5f)));
    shader->setVec3("spotLight.position", camera->getPosition());
    shader->setVec3("spotLight.direction", camera->getDirection());
          
    shader->setVec3("viewPos", camera->getPosition());

    // camera/view transformation
    glm::mat4 projection = window->getProjectionMatrix();
    shader->setMat4("projection", projection);
    glm::mat4 view = camera->getViewMatrix();
    shader->setMat4("view", view);

    // render boxes
    glBindVertexArray(VAO);
    for (unsigned int i = 0; i < 10; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        float angle = 20.0f * i;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // render the lamp
    lightShader->use();
    lightShader->setMat4("projection", projection);
    lightShader->setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pointLigthPos);
    model = glm::scale(model, glm::vec3(0.2f));
    lightShader->setMat4("model", model);
    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void ShaderTestScene::configureData()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // bind the VAO first, then bind and set vertex buffer(s), and then configure vertex attribute(s)
    glBindVertexArray(VAO);
    // copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coordinates attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
