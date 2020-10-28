#include "ModelTestScene.h"
#include "../Engine/Window.h"
#include "../Engine/Shader.h"
#include "../Engine/Model.h"

ModelTestScene::ModelTestScene(Window* _window) : Scene(_window) 
{
    shader = new Shader("Shaders/shader.vert", "Shaders/simpleShader.frag");
    backpackModel = new Model("Models/backpack/backpack.obj");
}

ModelTestScene::~ModelTestScene()
{
    delete shader;
    delete backpackModel;
}

void ModelTestScene::update()
{
    shader->use();
    shader->setMat4("projection", window->getProjectionMatrix());
    shader->setMat4("view", window->getCamera()->getViewMatrix());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f));
    model = glm::scale(model, glm::vec3(1.0f));
    shader->setMat4("model", model);
    backpackModel->draw(*shader);
}
