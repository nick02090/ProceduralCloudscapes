#ifndef COLOR_ENVIRONMENT_H
#define COLOR_ENVIRONMENT_H

#include "Environment.h"
#include "../Color.h"

struct ColorData : EnvironmentData {
	Color color;
};

class ColorEnvironment : public Environment {
public:
	ColorEnvironment();
	~ColorEnvironment() override;

	void update() override;
};

#endif // !COLOR_ENVIRONMENT_H
