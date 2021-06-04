#include <iostream>
#include <imgui.h>

#include "Window.h"
#include "Camera.h"
#include "Utilities.h"

Camera* Window::camera = new Camera(glm::vec3(0.0f, 10.0f, 0.0f));

bool Window::firstMouse = true;
bool Window::mouseCursorDisabled = false;
float Window::lastX = WINDOW_WIDTH / 2.0;
float Window::lastY = WINDOW_HEIGHT / 2.0;
std::vector<KeyReactor*> Window::keyReactors = std::vector<KeyReactor*>();

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
    if (updateViewport)
    {
        int viewportWidth, viewportHeight;
        glfwGetFramebufferSize(glfwWindow, &viewportWidth, &viewportHeight);
        glViewport(0, 0, viewportWidth, viewportHeight);
        width = static_cast<size_t>(viewportWidth);
        height = static_cast<size_t>(viewportHeight);
        updateViewport = false;
    }
    if (showGUI)
        gui->show();
    else
        gui->hide();
    gui->update();
}

void Window::calculateDeltaTime()
{
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

bool Window::isFullScreen()
{
    return glfwGetWindowMonitor(glfwWindow) != nullptr;
}

void Window::setFullScreen(bool fullScreen)
{
    if (isFullScreen() == fullScreen)
        return;

    if (fullScreen)
    {
        // get resolution of monitor
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        // switch to full screen
        glfwSetWindowMonitor(glfwWindow, glfwMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else
    {
        // restore initial window size and position
        glfwSetWindowMonitor(glfwWindow, nullptr, 10, 40, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    }

    updateViewport = true;
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
    const float padding = 10.0f;
    static int corner = 2;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (corner != -1)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - padding) : (work_pos.x + padding);
        window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - padding) : (work_pos.y + padding);
        window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    if (infoType == 0)
        ImGui::SetNextWindowSize(ImVec2(555, 155));
    else
        ImGui::SetNextWindowSize(ImVec2(555, 55));
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Procedural cloudscapes", &showGUI, window_flags))
    {
        if (infoType == 0) {

            ImGui::TextWrapped("Press ESC to exit the application.");

            ImGui::TextWrapped("To enable/disable full screen press F button on the keyboard.");

            ImGui::TextWrapped("To show/hide all GUI elements press H button on the keyboard.");

            ImGui::TextWrapped("To enable/disable mouse cursor press TAB button on the keyboard.");

            ImGui::TextWrapped("Move using WASD keys, mouse to look around, and scroll wheel to zoom IN/OUT.");

            ImGui::TextWrapped("Right click on this window to change its position, size and/or to hide it.");

            ImGui::Separator();

        }

        ImGui::TextWrapped("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::PushItemWidth(400); // Sets slider size
        float movementSpeed = camera->getMovementSpeed();
        ImGui::SliderFloat("Movement speed", &movementSpeed, 1.f, 10000.f);
        camera->setMovementSpeed(movementSpeed);

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
            if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
            if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
            ImGui::Separator();
            ImGui::RadioButton("Full info", &infoType, 0);
            ImGui::RadioButton("Small info", &infoType, 1);
            ImGui::Separator();
            if (ImGui::MenuItem("Hide")) showGUI = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

glm::mat4 Window::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(camera->getZoom()), (float)width / (float)height, NEAR, FAR);
}

void Window::resize(int cx, int cy)
{
    updateViewport = true;
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
    glfwSetWindowSizeCallback(window, resizeCallback);

    glfwMonitor = glfwGetPrimaryMonitor();
    updateViewport = true;

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

    // Switch fullscreen mode
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        void* ptr = glfwGetWindowUserPointer(window);
        if (Window* wndPtr = static_cast<Window*>(ptr)) {
            wndPtr->setFullScreen(!wndPtr->isFullScreen());
        }
    }

    // Switch GUI mode
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        void* ptr = glfwGetWindowUserPointer(window);
        if (Window* wndPtr = static_cast<Window*>(ptr)) {
            wndPtr->showGUI = !wndPtr->showGUI;
        }
    }

    // Do every other key reaction
    for (KeyReactor* reactor : keyReactors) {
        reactor->react(window, key, scancode, action, mods);
    }
}

void Window::resizeCallback(GLFWwindow* window, int cx, int cy)
{
    void* ptr = glfwGetWindowUserPointer(window);
    if (Window* wndPtr = static_cast<Window*>(ptr))
        wndPtr->resize(cx, cy);
}
