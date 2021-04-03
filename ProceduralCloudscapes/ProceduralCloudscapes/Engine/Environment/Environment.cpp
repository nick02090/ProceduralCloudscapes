#include "Environment.h"
#include "ColorEnvironment.h"
#include "GradientEnvironment.h"
#include "SkyboxEnvironment.h"

#include <iostream>

Environment* Environment::createEnvironment(EnvironmentType environmentType)
{
	Environment* env = nullptr;
	switch (environmentType)
	{
	case EnvironmentType::UNINITIALIZED:
		std::cout << "Environment cannot be UNINITIALIZED!" << std::endl;
		break;
	case EnvironmentType::Color:
		env = new ColorEnvironment();
		break;
	case EnvironmentType::Gradient:
		env = new GradientEnvironment();
		break;
	case EnvironmentType::Skybox:
		env = new SkyboxEnvironment();
		break;
	default:
		std::cout << "Invalid environment type!" << std::endl;
		break;
	}
	return env;
}