#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <iostream>

#include "Window.h"
#include "SceneObject.h"
#include "FrameBufferObject.h"
#include "Environment/Environment.h"

class Scene {
public:
	Scene(Window* _window, const char* _name, EnvironmentType environmentType) : window(_window), name(_name) {
		// create the environment
		environment = Environment::createEnvironment(environmentType);
		// set the window title
		window->setTitle(name);
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
	const char* name;

	template<class T, typename std::enable_if<!std::is_same<T, Environment>::value, int>::type = 0>
	T* getEnvironment() {
		// Check the type of the given T class
		if (!Environment::checkType<T>(environment->getType())) {
			std::cout << "ERROR::SCENE::getEnvironment() Wrong class name (" << typeid(T).name() << ") for the environment has been used! ";
			std::cout << "This scene (" << name << ") has an environment of type [" << environment->getType_c() << "]." << std::endl;
			return nullptr;
		}
		// Cast the environment to valid derived class
		return static_cast<T*>(environment);
	}

private:
	Environment* environment;
};

#endif // !SCENE_H
