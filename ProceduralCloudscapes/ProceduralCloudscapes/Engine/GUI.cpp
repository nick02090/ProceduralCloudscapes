#include "GUI.h"
#include "Window.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

GUI::GUI(Window& window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window.getGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

GUI::~GUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::update()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Update all the children elements
    for (GUIBuilder* builder : builders) {
        builder->buildGUI();
    }

    // Render at the end of all the children updates
    ImGui::Render();
}

void GUI::draw()
{
    // Draw should be called at the end of the scene rendering
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::subscribe(GUIBuilder* builder)
{
    builders.push_back(builder);
}
