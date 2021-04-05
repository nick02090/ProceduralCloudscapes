#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string.h>
#include <iostream>

#include "../Window.h"

enum class EnvironmentType
{
	UNINITIALIZED,
	Color,
	Gradient,
	Skybox
};

struct EnvironmentData {
};

class Environment {
public:
	Environment(Window* _window) : window(_window) {}
	virtual ~Environment() {
		delete data;
	};

	static Environment* createEnvironment(EnvironmentType environmentType, Window* _window);

	void draw() {
		// clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// render the environment
		update();
	}

	virtual void update() = 0;

	EnvironmentType getType() const { return type; }
	const char* getType_c() const {
		switch (type)
		{
		case EnvironmentType::UNINITIALIZED:
			return "UNINITIALIZED";
			break;
		case EnvironmentType::Color:
			return "Color";
			break;
		case EnvironmentType::Gradient:
			return "Gradient";
			break;
		case EnvironmentType::Skybox:
			return "Skybox";
			break;
		default:
			return nullptr;
			break;
		}
	}

	template<class T, typename std::enable_if<!std::is_same<T, Environment>::value, int>::type = 0>
	static bool checkType(EnvironmentType type) {
		// Set the object type to initial value
		EnvironmentType objType = EnvironmentType::UNINITIALIZED;
		// Check which type is it
		const char* objName = typeid(T).name();
		if (strcmp(objName, "class ColorEnvironment") == 0) {
			objType = EnvironmentType::Color;
		}
		else if (strcmp(objName, "class GradientEnvironment") == 0) {
			objType = EnvironmentType::Gradient;
		}
		else if (strcmp(objName, "class SkyboxEnvironment") == 0) {
			objType = EnvironmentType::Skybox;
		}
		// Compare calculated type with the given one
		if (objType == type) {
			return true;
		}
		return false;
	}
protected:
	EnvironmentType type = EnvironmentType::UNINITIALIZED;
	EnvironmentData* data = nullptr;
	Window* window;
};

#endif // !ENVIRONMENT_H
