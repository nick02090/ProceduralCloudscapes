#ifndef CLOUDS_H
#define CLOUDS_H

#include "../Engine/SceneObject.h"

class Clouds : public SceneObject, public GUIBuilder {
public:
	Clouds(Window* _window);
	~Clouds();

	void update() override;
	void buildGUI() override;

private:
	void generateTextures();
};

#endif // !CLOUDS_H
