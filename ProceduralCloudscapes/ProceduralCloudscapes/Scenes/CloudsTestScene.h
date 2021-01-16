#ifndef CLOUDS_TEST_SCENE
#define CLOUDS_TEST_SCENE

#include "../Engine/Scene.h"

class CloudsTestScene : public Scene
{
public:
	CloudsTestScene(Window* _window);
	~CloudsTestScene();

	virtual void update();

private:
};

#endif // !CLOUDS_TEST_SCENE
