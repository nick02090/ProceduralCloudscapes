#ifndef RAYMARCH_TEST_SCENE
#define RAYMARCH_TEST_SCENE

#include "../Engine/Scene.h"

class Shader;

class RaymarchTestScene : public Scene
{
public:
    RaymarchTestScene(Window* _window);
    ~RaymarchTestScene();

    virtual void update();

private:
    void configureData();

    Shader* shader;
    Shader* screenShader;

    unsigned int framebuffer;
    unsigned int textureColorbuffer;

    unsigned int quadVAO, quadVBO;
    unsigned int rbo;

    float quadVertices[24] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
};

#endif // !RAYMARCH_TEST_SCENE

