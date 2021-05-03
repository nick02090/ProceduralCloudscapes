#ifndef SCREEN_SHADER_H
#define SCREEN_SHADER_H

#include "Shader.h"

class Texture;

class ScreenShader {
public:
	ScreenShader(const char* fragShaderPath, const char* vertShaderPath = "Shaders/Screen/shader.vert");
	~ScreenShader();

	void draw(const Texture& texture);

	// GETTERS
	Shader* getShader() { return shader; }
private:
	unsigned int quadVAO, quadVBO;

	Shader* shader;

	void configureData();
};

#endif // !SCREEN_SHADER_H
