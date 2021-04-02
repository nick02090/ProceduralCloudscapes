#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "Window.h"

class SceneObject {
public:
	SceneObject(Window* _window) : window(_window) {};
	virtual ~SceneObject() {};

	void draw() {
		// update the scene object
		update();
	}

	virtual void update() = 0;

protected:
	Window* window;
};

#endif // !SCENE_OBJECT_H
