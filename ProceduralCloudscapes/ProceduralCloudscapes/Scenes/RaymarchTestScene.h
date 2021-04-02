#ifndef RAYMARCH_TEST_SCENE
#define RAYMARCH_TEST_SCENE

#include "../Engine/Scene.h"

class Shader;
class ScreenShader;
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
    ScreenShader* screenShader;

    FrameBufferObject* framebuffer;
};

#endif // !RAYMARCH_TEST_SCENE

