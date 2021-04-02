// ProceduralCloudscapes.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


// NOTE: Always include GLAD before other header files that require OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "Engine/Window.h"
#include "Engine/Scene.h"
#include "Scenes/ShaderTestScene.h"
#include "Scenes/FramebufferTestScene.h"
#include "Scenes/RaymarchTestScene.h"

int main()
{
    // configure global stbi state
    stbi_set_flip_vertically_on_load(true);

    // create a window for rendering
    Window window("Raymarch test scene");

    // load a scene that will show up in the window
    Scene* scene = new RaymarchTestScene(&window);

    // render loop
    while (window.isRunning())
    {
        // update window every frame
        window.update();

        // process input on the window
        window.processInput();

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene->draw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved, etc.)
        glfwSwapBuffers(window.getGLFWWindow());
        glfwPollEvents();
    }

    delete scene;

    return 0;
}