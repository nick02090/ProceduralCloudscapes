#ifndef SCENE_H
#define SCENE_H

#include "Window.h"

class Scene {
public:
	Scene(Window* _window) : window(_window) {};
	virtual ~Scene() {};

	virtual void update() = 0;

protected:
	Window* window;
};

#endif // !SCENE_H
