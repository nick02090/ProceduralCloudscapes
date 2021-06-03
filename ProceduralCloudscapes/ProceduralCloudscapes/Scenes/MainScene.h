#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include "../Engine/Scene.h"

class MainScene : public Scene 
{
public:
	MainScene(Window* _window);
	~MainScene();

	void update() override;

};

#endif // !MAIN_SCENE_H
