#ifndef PBR_TEST_SCENE
#define PBR_TEST_SCENE

#include "../Engine/Scene.h"
#include "../Engine/Texture.h"
#include "../Engine/Shader.h"

class PBRTestScene : public Scene 
{
public:
	PBRTestScene(Window* _window);
	~PBRTestScene();

	void update() override;
};

#endif // !PBR_TEST_SCENE
