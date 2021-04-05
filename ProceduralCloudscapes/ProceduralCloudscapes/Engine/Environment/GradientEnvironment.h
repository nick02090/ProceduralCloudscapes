#ifndef GRADIENT_ENVIRONMENT_H
#define GRADIENT_ENVIRONMENT_H

#include "Environment.h"

struct GradientEnvironmentData : EnvironmentData {

};

class GradientEnvironment : public Environment {
public:
	GradientEnvironment();
	~GradientEnvironment();

	void update() override;
};

#endif // !GRADIENT_ENVIRONMENT_H
