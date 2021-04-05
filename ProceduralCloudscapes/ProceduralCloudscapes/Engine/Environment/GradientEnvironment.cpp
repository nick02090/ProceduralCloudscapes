#include "GradientEnvironment.h"

GradientEnvironment::GradientEnvironment()
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
