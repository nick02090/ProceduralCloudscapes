#ifndef SPHERE_H
#define SPHERE_H

#include "../Engine/SceneObject.h"
#include "../Engine/Shader.h"
#include "../Engine/Texture.h"

class Sphere : public SceneObject 
{
public:
	Sphere(Window* _window, const char* texturesPath, glm::vec3 _translate = glm::vec3(0.f));
	~Sphere();

	void update() override;

private:
	Shader* shader = nullptr;

	Texture* albedoTex = nullptr;
	Texture* normalTex = nullptr;
	Texture* metallicTex = nullptr;
	Texture* roughnessTex = nullptr;
	Texture* aoTex = nullptr;

	glm::vec3 translate;

	unsigned int sphereVAO = 0;
	size_t indexCount = 0;

	void configureData();
};

#endif // !SPHERE_H
