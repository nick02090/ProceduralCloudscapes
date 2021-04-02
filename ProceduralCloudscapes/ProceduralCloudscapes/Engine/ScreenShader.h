#ifndef SCREEN_SHADER_H
#define SCREEN_SHADER_H

#include "Shader.h"

class ScreenShader {
public:
	ScreenShader(const char* fragShaderPath);
	~ScreenShader();

	void draw(unsigned int texID);

	// GETTERS
	Shader* getShader() { return shader; }
private:
	unsigned int quadVAO, quadVBO;

	Shader* shader;

	void configureData();
};

#endif // !SCREEN_SHADER_H
