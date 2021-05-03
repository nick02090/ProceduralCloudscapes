#ifndef PLANE_TEXTURE_H
#define PLANE_TEXTURE_H

#include "../Engine/SceneObject.h"
#include "../Engine/Shader.h"

class Texture;

enum class TextureChannel {
	R = 0,
	G = 1,
	B = 2,
	A = 3
};

class PlaneTexture : public SceneObject {
public:
	PlaneTexture(Window* _window, Texture* _texture, glm::vec3 _offset = glm::vec3(0.f), TextureChannel _channel = TextureChannel::R);
	~PlaneTexture();

	void update() override;
private:
	void configureData(glm::vec3 offset);

	Shader* shader;

	unsigned int planeVAO, planeVBO;

	Texture* texture;
	TextureChannel channel;
};

#endif // !PLANE_TEXTURE_H
