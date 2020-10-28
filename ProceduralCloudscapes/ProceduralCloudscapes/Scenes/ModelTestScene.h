#ifndef MODEL_TEST_SCENE
#define MODEL_TEST_SCENE

#include "../Engine/Scene.h"

class Shader;
class Model;

class ModelTestScene : public Scene
{
public:
	ModelTestScene(Window* _window);
	~ModelTestScene();

	virtual void update();

private:
	Shader* shader;
	Model* backpackModel;
};

#endif // !MODEL_TEST_SCENE
