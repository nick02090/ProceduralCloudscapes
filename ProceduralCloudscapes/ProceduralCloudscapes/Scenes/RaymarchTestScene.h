#ifndef RAYMARCH_TEST_SCENE
#define RAYMARCH_TEST_SCENE

#include "../Engine/Scene.h"

class Shader;
class FrameBufferObject;

class RaymarchTestScene : public Scene
{
public:
    RaymarchTestScene(Window* _window);
    ~RaymarchTestScene();

    void update() override;

private:
    void configureData();

    Shader* shader;
    Shader* screenShader;

    FrameBufferObject* framebuffer;

    unsigned int quadVAO, quadVBO;

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

