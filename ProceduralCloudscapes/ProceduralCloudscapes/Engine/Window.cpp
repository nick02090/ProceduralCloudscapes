#include <iostream>

#include "Window.h"
#include "Camera.h"
#include "Utilities.h"

Camera* Window::camera = new Camera(glm::vec3(0.0f, 1.0f, 0.0f));

bool Window::firstMouse = true;
float Window::lastX = WINDOW_WIDTH / 2.0;
float Window::lastY = WINDOW_HEIGHT / 2.0;

Window::Window(const char* title, size_t _width, size_t _height) : width(_width), height(_height)
{
    glfwWindow = initGLFW(title);
    initGLAD();
    initOPENGL();
}

Window::~Window()
{
    // GLFW: terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
}

void Window::update()
{
    calculateDeltaTime();
}

void Window::calculateDeltaTime()
{
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Window::processInput()
{
    if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(glfwWindow, true);
    const float cameraSpeed = 5.0f * deltaTime;
    if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Forward, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Backward, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_Q) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Down, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_E) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Up, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Left, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Right, deltaTime);
}

glm::mat4 Window::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(camera->getZoom()), (float)width / (float)height, 0.1f, 100.0f);
}

GLFWwindow* Window::initGLFW(const char* title)
{
    // Initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW window creation
    GLFWwindow* window = glfwCreateWindow((int)width, (int)height, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetWindowUserPointer(window, this);

    return window;
}

void Window::initGLAD()
{
    // GLAD: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }
}

void Window::initOPENGL()
{
    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(glfwWindow, mouseCallback);
    glfwSetScrollCallback(glfwWindow, scrollCallback);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Window::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // reversed since y-coordinates range from bottom to top
    lastX = (float)xpos;
    lastY = (float)ypos;

    camera->processMouseMovement(xoffset, yoffset);

}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera->processMouseScroll((float)yoffset);
}
