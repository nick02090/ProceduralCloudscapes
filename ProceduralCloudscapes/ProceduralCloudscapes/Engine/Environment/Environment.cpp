#include "Environment.h"
#include "ColorEnvironment.h"
#include "GradientEnvironment.h"
#include "SkyboxEnvironment.h"

Environment* Environment::createEnvironment(EnvironmentType environmentType, Window* _window)
{
	Environment* env = nullptr;
	switch (environmentType)
	{
	case EnvironmentType::UNINITIALIZED:
		std::cout << "Environment cannot be UNINITIALIZED!" << std::endl;
		break;
	case EnvironmentType::Color:
		env = new ColorEnvironment(_window);
		break;
	case EnvironmentType::Gradient:
		env = new GradientEnvironment(_window);
		break;
	case EnvironmentType::Skybox:
		env = new SkyboxEnvironment(_window);
		break;
	default:
		std::cout << "Invalid environment type!" << std::endl;
		break;
	}
	return env;
}