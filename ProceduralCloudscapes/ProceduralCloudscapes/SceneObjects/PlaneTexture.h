#ifndef PLANE_TEXTURE_H
#define PLANE_TEXTURE_H

#include "../Engine/SceneObject.h"
#include "../Engine/Shader.h"

class Texture;

class PlaneTexture : public SceneObject {
public:
	PlaneTexture(Window* _window, Texture* _texture, glm::vec3 _offset = glm::vec3(0.f));
	~PlaneTexture();

	void update() override;
private:
	void configureData(glm::vec3 offset);

	Shader* shader;

	unsigned int planeVAO, planeVBO;

	Texture* texture;
};

#endif // !PLANE_TEXTURE_H
