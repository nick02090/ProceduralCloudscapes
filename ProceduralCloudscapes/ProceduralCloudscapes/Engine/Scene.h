#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "Window.h"
#include "SceneObject.h"
#include "FrameBufferObject.h"
#include "Environment/Environment.h"

class Scene {
public:
	Scene(Window* _window, EnvironmentType environmentType) : window(_window) {
		// create the environment
		environment = Environment::createEnvironment(environmentType);
	};
	virtual ~Scene() {
		// delete created scene objects
		for (auto sceneObject : sceneObjects)
		{
			delete sceneObject;
		}
		// delete the environment
		delete environment;
	}

	void draw() {
		// render the environment
		environment->draw();
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
	Environment* environment;
};

#endif // !SCENE_H
