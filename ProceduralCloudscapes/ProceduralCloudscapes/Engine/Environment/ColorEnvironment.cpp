#include "ColorEnvironment.h"

#include <imgui.h>

ColorEnvironment::ColorEnvironment(Window* _window) : Environment(_window)
{
	// Initialize member variables
	type = EnvironmentType::Color;
	data = new ColorEnvironmentData();
}

ColorEnvironment::~ColorEnvironment()
{
}

void ColorEnvironment::update()
{
	// cast the color data
	ColorEnvironmentData* colorEnvironmentData = static_cast<ColorEnvironmentData*>(data);
	// get the color
	Color color = colorEnvironmentData->color;
	// draw the selected background color
	glClearColor(color.getRf(), color.getGf(), color.getBf(), 1.0f);
}

void ColorEnvironment::extendGUI()
{
	// Color
	ImVec4 color = getColor().toIMGUI();
	ImGui::ColorEdit3("Color", (float*)&color);
	setColor(Color::fromIMGUI(color));
}
