#ifndef TERRAIN_H
#define TERRAIN_H

#include "../Engine/SceneObject.h"

class Terrain : public SceneObject, public GUIBuilder {
public:
	Terrain(Window* _window);
	~Terrain();

	void update() override;
	void buildGUI() override;
};

#endif // !TERRAIN_H
