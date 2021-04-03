#include "ColorEnvironment.h"

ColorEnvironment::ColorEnvironment()
{
	// Initialize member variables
	type = EnvironmentType::Color;
}

ColorEnvironment::~ColorEnvironment()
{
}

void ColorEnvironment::update()
{
	// draw the selected background color
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
}
