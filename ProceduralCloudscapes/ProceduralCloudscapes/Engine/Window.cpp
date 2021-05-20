#include <iostream>
#include <imgui.h>

#include "Window.h"
#include "Camera.h"
#include "Utilities.h"

Camera* Window::camera = new Camera(glm::vec3(0.0f, 1.0f, 0.0f));

bool Window::firstMouse = true;
bool Window::mouseCursorDisabled = false;
float Window::lastX = WINDOW_WIDTH / 2.0;
float Window::lastY = WINDOW_HEIGHT / 2.0;

Window::Window(const char* title, size_t _width, size_t _height) : width(_width), height(_height)
{
    glfwWindow = initGLFW(title);
    initGLAD();
    initOPENGL();
    // create gui
    gui = new GUI(*this);
    // subscribe to gui
    gui->subscribe(this);
}

Window::~Window()
{
    // delete gui
    delete gui;
    // GLFW: terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
}

void Window::update()
{
    calculateDeltaTime();
    gui->update();
}

void Window::calculateDeltaTime()
{
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Window::processInput()
{
    // quit application
    if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(glfwWindow, true);

    // disable movement when cursor is disabled
    if (mouseCursorDisabled) {
        return;
    }

    // movement
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

void Window::buildGUI()
{
    ImGui::Begin("Main");

    ImGui::TextWrapped("Move using WASD keys and mouse to look around.");

    ImGui::TextWrapped("To enable/disable mouse cursor press TAB button on the keyboard.");

    ImGui::TextWrapped("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    float movementSpeed = camera->getMovementSpeed();
    ImGui::SliderFloat("Movement speed", &movementSpeed, 1.f, 10000.f);
    camera->setMovementSpeed(movementSpeed);

    ImGui::End();
}

glm::mat4 Window::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(camera->getZoom()), (float)width / (float)height, NEAR, FAR);
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
    glfwSetKeyCallback(window, keyCallback);
    glfwSetWindowUserPointer(window, this);
    glfwSetWindowPos(window, 10, 40);

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

    if (!mouseCursorDisabled)
        camera->processMouseMovement(xoffset, yoffset);
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // disable movement when cursor is disabled
    if (mouseCursorDisabled) {
        return;
    }

    camera->processMouseScroll((float)yoffset);
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Switch cursor state
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, (mouseCursorDisabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL));
        mouseCursorDisabled = !mouseCursorDisabled;
        if (!mouseCursorDisabled) {
            firstMouse = true;
        }
    }
}
