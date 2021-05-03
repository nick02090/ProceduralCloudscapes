#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include "Window.h"

class Scene;

class SceneObject {
public:
	SceneObject(Window* _window) : window(_window) {};
	virtual ~SceneObject() {};

	void draw() {
		// update the scene object
		update();
	}

	virtual void update() = 0;

protected:
	Window* window;
	Scene* scene = nullptr;

	friend class Scene;
	inline void setScene(Scene* _scene) { scene = _scene; }
	inline Scene* getScene() const { return scene; }
};

#endif // !SCENE_OBJECT_H
