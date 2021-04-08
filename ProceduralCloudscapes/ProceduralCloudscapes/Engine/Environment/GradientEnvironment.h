#ifndef GRADIENT_ENVIRONMENT_H
#define GRADIENT_ENVIRONMENT_H

#include "Environment.h"

struct GradientEnvironmentData : EnvironmentData {

};

class GradientEnvironment : public Environment {
public:
	GradientEnvironment(Window* _window);
	~GradientEnvironment();

	void update() override;
	void extendGUI() override;
};

#endif // !GRADIENT_ENVIRONMENT_H
