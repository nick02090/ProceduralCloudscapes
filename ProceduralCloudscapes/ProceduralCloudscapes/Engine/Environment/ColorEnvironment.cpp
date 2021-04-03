#include "ColorEnvironment.h"

ColorEnvironment::ColorEnvironment()
{
	// Initialize member variables
	type = EnvironmentType::Color;
	data = new ColorData();
}

ColorEnvironment::~ColorEnvironment()
{
}

void ColorEnvironment::update()
{
	// cast the color data
	ColorData* colorData = static_cast<ColorData*>(data);
	// get the color
	Color color = colorData->color;
	// draw the selected background color
	glClearColor(color.getRf(), color.getGf(), color.getBf(), 1.0f);
}
