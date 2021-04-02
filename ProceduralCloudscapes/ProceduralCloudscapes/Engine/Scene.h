#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "Window.h"
#include "SceneObject.h"

class Scene {
public:
	Scene(Window* _window) : window(_window) {};
	virtual ~Scene() {
		for (auto sceneObject : sceneObjects)
		{
			delete sceneObject;
		}
	}

	void draw() {
		// update the scene
		update();
		// draw every scene object
		for (auto sceneObject : sceneObjects)
		{
			sceneObject->draw();
		}
	}

	virtual void update() = 0;

protected:
	Window* window;
	std::vector<SceneObject*> sceneObjects;
};

#endif // !SCENE_H
