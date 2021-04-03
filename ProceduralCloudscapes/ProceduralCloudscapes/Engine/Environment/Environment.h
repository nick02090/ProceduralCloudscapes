#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
	virtual ~Environment() {};

	static Environment* createEnvironment(EnvironmentType environmentType);

	void draw() {
		// clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// render the environment
		update();
	}

	virtual void update() = 0;

	// SETTERS
	void setData(EnvironmentData _data) { data = _data; }

	// GETTERS
	EnvironmentData getData() const { return data; }
protected:
	EnvironmentType type = EnvironmentType::UNINITIALIZED;
	EnvironmentData data;
};

#endif // !ENVIRONMENT_H
