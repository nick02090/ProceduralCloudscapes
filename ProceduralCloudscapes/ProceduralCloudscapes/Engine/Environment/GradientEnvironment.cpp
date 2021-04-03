#include "GradientEnvironment.h"

GradientEnvironment::GradientEnvironment()
{
	// Initialize member variables
	type = EnvironmentType::Gradient;
	data = new GradientData();
}

GradientEnvironment::~GradientEnvironment()
{
}

void GradientEnvironment::update()
{
}
