#ifndef COLOR_ENVIRONMENT_H
#define COLOR_ENVIRONMENT_H

#include "Environment.h"
#include "../Color.h"

struct ColorEnvironmentData : EnvironmentData {
	Color color;
};

class ColorEnvironment : public Environment {
public:
	ColorEnvironment(Window* _window);
	~ColorEnvironment() override;

	void update() override;

	// SETTERS

	void setColor(Color color) { static_cast<ColorEnvironmentData*>(data)->color = color; }
	void setColor(int r, int g, int b) { static_cast<ColorEnvironmentData*>(data)->color.set(r, g, b); }
	void setColor(float r, float g, float b) { static_cast<ColorEnvironmentData*>(data)->color.setf(r, g, b); }

	// GETTERS

	Color getColor() const { return static_cast<ColorEnvironmentData*>(data)->color; }
};

#endif // !COLOR_ENVIRONMENT_H
