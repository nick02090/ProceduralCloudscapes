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
		environment = Environment::createEnvironment(environmentType, window);
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
		// clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// render the environment (environment is rendered in main buffer and in seperate texture)
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

	Texture* getEnvironmentTexture() const { return environment->getTexture(); }

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

protected:
	Window* window;
	const char* name;

	void addSceneObject(SceneObject* sceneObject) {
		// Add scene object to the list
		sceneObjects.push_back(sceneObject);
		// Set scene object scene to this
		sceneObject->setScene(this);
	}

private:
	Environment* environment;
	std::vector<SceneObject*> sceneObjects;
};

#endif // !SCENE_H
