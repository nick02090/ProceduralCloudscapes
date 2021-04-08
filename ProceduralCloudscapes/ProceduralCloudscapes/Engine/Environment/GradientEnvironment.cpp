#include "GradientEnvironment.h"

GradientEnvironment::GradientEnvironment(Window* _window) : Environment(_window)
{
	// Initialize member variables
	type = EnvironmentType::Gradient;
	data = new GradientEnvironmentData();
}

GradientEnvironment::~GradientEnvironment()
{
}

void GradientEnvironment::update()
{
}

void GradientEnvironment::extendGUI()
{
}
