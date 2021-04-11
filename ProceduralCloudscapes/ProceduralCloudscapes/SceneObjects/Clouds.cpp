#include "Clouds.h"

#include "../Engine/Utilities.h"
#include "PlaneTexture.h"

Clouds::Clouds(Window* _window) : SceneObject(_window)
{
	generateTextures();

	// Test plane textures for checking cloud textures calculation
	planeTexture = new PlaneTexture(window, perlinWorleyTex);
}

Clouds::~Clouds()
{
	delete perlinWorleyTex;
	delete worleyTex;
	delete curlTex;
}

void Clouds::update()
{
	planeTexture->update();
	// wait for all the memory stores, loads, textures fetches, vertex fetches
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Clouds::buildGUI()
{
}

void Clouds::generateTextures()
{
	// =============================================
	// 1st 3D texture (Perlin-Worley) (128^3) RGBA
	// =============================================

	// create shader
	Shader* perlinWorleyShader = new Shader();
	perlinWorleyShader->attachShader("Shaders/Noise/perlinWorley.comp", ShaderInfo(ShaderType::kCompute));
	perlinWorleyShader->linkProgram();

	// create texture
	perlinWorleyTex = new Texture(TextureType::threeDimensional, 128, 4, true);

	// configure shader
	perlinWorleyShader->use();
	glActiveTexture(GL_TEXTURE0);
	perlinWorleyShader->setInt("perlinWorleyTex", 0);
	glBindTexture(GL_TEXTURE_3D, perlinWorleyTex->ID);
	glBindImageTexture(0, perlinWorleyTex->ID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	glDispatchCompute(INT_CEIL(128, 4), INT_CEIL(128, 4), INT_CEIL(128, 4));

	// delete shader
	delete perlinWorleyShader;

	// =============================================
	// 2nd 3D texture (Worley) (32^3) RGB
	// =============================================



	// =============================================
	// 1st 2D texture (Curl) (128^2) RGB
	// =============================================
}
