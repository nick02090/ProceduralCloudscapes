#ifndef WINDOW_H
#define WINDOW_H

// NOTE: Always include GLAD before other header files that require OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "Camera.h"

// default window settings
const size_t WINDOW_WIDTH = 800;
const size_t WINDOW_HEIGHT = 600;

enum class Callback
{
	FramebufferSize,
	Mouse,
	Scroll
};

class Window {
public:
	/// <summary>
	/// Initializes new window as well as all necessary libs for rendering (GLFW, GLAD, OPENGL).
	/// </summary>
	/// <param name="title">Title of the window that is shown in the upper left corner.</param>
	/// <param name="_width">Width of the window.</param>
	/// <param name="_height">Height of the window.</param>
	Window(const char* title, size_t _width = WINDOW_WIDTH, size_t _height = WINDOW_HEIGHT);
	~Window();

	/// <summary>
	/// Should be called every frame in main engine loop.
	/// This updates all necessary window settings.
	/// </summary>
	void update();
	/// <summary>
	/// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
	/// </summary>
	void processInput();

	/// <summary>
	/// Returns GLFW Window object.
	/// </summary>
	/// <returns></returns>
	inline GLFWwindow* getGLFWWindow() const { return glfwWindow; }
	/// <summary>
	/// Determines whether the window is still running (rendering).
	/// </summary>
	/// <returns></returns>
	inline bool isRunning() const { return !glfwWindowShouldClose(glfwWindow); }
	inline size_t getWidth() const { return width; }
	inline size_t getHeight() const { return height; }
	inline Camera* getCamera() const { return camera; }
	glm::mat4 getProjectionMatrix() const;

private:
	/// <summary>
	/// Initializes GLFW.
	/// </summary>
	/// <param name="title"></param>
	/// <returns></returns>
	GLFWwindow* initGLFW(const char* title);
	/// <summary>
	/// Initializes GLAD.
	/// </summary>
	void initGLAD();
	/// <summary>
	/// Initializes global OPENGL settings.
	/// </summary>
	void initOPENGL();

	/// <summary>
	/// Whenever the window size changed (by OS or user resize) this callback function executes.
	/// </summary>
	/// <param name="window">GLFW Window object</param>
	/// <param name="width">New window width</param>
	/// <param name="height">New window height</param>
	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
	/// <summary>
	/// Each time the mouse moves this callback function executes.
	/// </summary>
	/// <param name="window">GLFW Window object</param>
	/// <param name="xpos">X position of the mouse on the window.</param>
	/// <param name="ypos">Y position of the mouse on the window.</param>
	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	/// <summary>
	/// Each time the scrool button on the mouse moves this callback function executes.
	/// </summary>
	/// <param name="window">GLFW Window object</param>
	/// <param name="xoffset">X offest of the scroll</param>
	/// <param name="yoffset">Y offset of the scoll</param>
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	/// <summary>
	/// Calculates current delta time based on the elapsed time from the last frame.
	/// </summary>
	void calculateDeltaTime();

	size_t width, height;
	GLFWwindow* glfwWindow;

	static Camera* camera;

	// Used for calculating current delta frame time.
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// Ensures that the first mouse movement isn't a "jump".
	static float lastX, lastY;
	static bool firstMouse;
};

#endif // !WINDOW_H
