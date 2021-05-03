#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string.h>
#include <iostream>
#include <imgui.h>

#include "../Window.h"
#include "../FrameBufferObject.h"
#include "../ScreenShader.h"
#include "../Texture.h"

enum class EnvironmentType
{
	UNINITIALIZED,
	Color,
	Gradient,
	Skybox
};

struct EnvironmentData {
};

class Environment : public GUIBuilder {
public:
	Environment(Window* _window) : window(_window) {
		// subscribe to GUI
		window->getGUI()->subscribe(this);

		// framebuffer configuration
		framebuffer = new FrameBufferObject();
		// create a color attachment texture
		framebuffer->attachColorTexture((unsigned int)window->getWidth(), (unsigned int)window->getHeight());
		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		framebuffer->attachDepthTexture((unsigned int)window->getWidth(), (unsigned int)window->getHeight());
		// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		if (!framebuffer->checkStatus())
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	virtual ~Environment() {
		delete data;
		delete framebuffer;
	};

	static Environment* createEnvironment(EnvironmentType environmentType, Window* _window);

	Texture* getTexture() const { return framebuffer->getColorTexture(0); }

	void draw() {
		// clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// bind the framebuffer
		framebuffer->bind();
		// clear its data before any rendering
		framebuffer->clear();

		// render the environment
		update();

		// unbind the buffer (set the default buffer)
		FrameBufferObject::unbind();

		// create a screen shader for rendering the buffer on the screen
		ScreenShader* screenShader = new ScreenShader("Shaders/Default/textureShader2D.frag");
		// disable depth test so screen-space quad isn't discarded due to depth test
		glDisable(GL_DEPTH_TEST);
		// draw the screen shader with the buffer texture
		screenShader->draw(*getTexture());
		// delete the created screen shader
		delete screenShader;
		// enable back depth test
		glEnable(GL_DEPTH_TEST);
	}

	virtual void update() = 0;

	void buildGUI() override {
		// create the environment window
		ImGui::Begin("Environment");

		// let the child expand the window
		extendGUI();

		// finish the window
		ImGui::End();
	}

	virtual void extendGUI() = 0;

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
	FrameBufferObject* framebuffer;
};

#endif // !ENVIRONMENT_H
