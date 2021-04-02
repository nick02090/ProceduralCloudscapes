#ifndef SKYBOX_TEST_SCENE
#define SKYBOX_TEST_SCENE

#include "../Engine/Scene.h"

class SkyboxTestScene : public Scene
{
public:
	SkyboxTestScene(Window* _window);
	~SkyboxTestScene();

	void update() override;
};


#endif // !SKYBOX_TEST_SCENE
