#ifndef CLOUDS_H
#define CLOUDS_H

#define INT_CEIL(n,d) (int)ceil((float)n/d)

#include "../Engine/SceneObject.h"
#include "../Engine/Texture.h"
#include "../Engine/Shader.h"

class PlaneTexture;

class Clouds : public SceneObject, public GUIBuilder {
public:
	Clouds(Window* _window);
	~Clouds();

	void update() override;
	void buildGUI() override;

private:
	void generateTextures();

	Texture* perlinWorleyTex = nullptr;
	Texture* worleyTex = nullptr;
	Texture* curlTex = nullptr;

	PlaneTexture* planeTexture = nullptr;
};

#endif // !CLOUDS_H
