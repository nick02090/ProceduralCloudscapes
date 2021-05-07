#include "PlaneTexture.h"

#include "../Engine/Texture.h"

PlaneTexture::PlaneTexture(Window* _window, Texture* _texture, glm::vec3 offset, TextureChannel _channel) : SceneObject(_window)
{
	// initialize member variables
	texture = _texture;
	channel = _channel;

	// create shader
	shader = new Shader();
	shader->attachShader("Shaders/Default/shader.vert", ShaderInfo(ShaderType::kVertex));
	if (texture->getType() == TextureType::twoDimensional)
		shader->attachShader("Shaders/Default/textureShader2D.frag", ShaderInfo(ShaderType::kFragment));
	else if (texture->getType() == TextureType::threeDimensional)
		shader->attachShader("Shaders/Default/textureShader3D.frag", ShaderInfo(ShaderType::kFragment));
	else
		std::cout << "ERROR::PLANETEXTURE Plane texture can be only made with 2D and 3D textures!" << std::endl;
	shader->linkProgram();

	// configure shader data
	configureData(offset);
}

PlaneTexture::~PlaneTexture()
{
	delete shader;
}

void PlaneTexture::update()
{
	Camera* camera = window->getCamera();

	shader->use();
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(camera->getZoom()), (float)window->getWidth() / (float)window->getHeight(), 0.1f, 100.0f);
	shader->setMat4("view", view);
	shader->setMat4("projection", projection);
	shader->setMat4("model", glm::mat4(1.0f));

	shader->setInt("channel", static_cast<int>(channel));
	shader->setSampler("texture1", *texture, 0);

	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void PlaneTexture::configureData(glm::vec3 offset)
{
	// assemble plane vertices
	float planeVertices[30] = {
		// positions          // texture Coords 
		-1.0f + offset.x,  0.0f + offset.y, -5.0f + offset.z,  0.0f, 0.0f,
		 1.0f + offset.x,  0.0f + offset.y, -5.0f + offset.z,  1.0f, 0.0f,
		 1.0f + offset.x,  2.0f + offset.y, -5.0f + offset.z,  1.0f, 1.0f,

		 1.0f + offset.x,  2.0f + offset.y, -5.0f + offset.z,  1.0f, 1.0f,
		-1.0f + offset.x,  2.0f + offset.y, -5.0f + offset.z,  0.0f, 1.0f,
		-1.0f + offset.x,  0.0f + offset.y, -5.0f + offset.z,  0.0f, 0.0f
	};

	// plane VAO
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// load textures
	shader->use();
	shader->setInt("texture1", 0);
}
