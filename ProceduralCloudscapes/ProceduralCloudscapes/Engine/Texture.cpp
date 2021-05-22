#include "Texture.h"

#include <iostream>
#include <stb_image.h>

Texture::Texture(TextureType _type, glm::vec3 _size, uint8_t nrChannels, bool is8bit)
{
	// initialize member variables
	size = _size;
	// create texture info
	info = new TextureInfo(_type);
	// create GL texture
	ID = generateGlTexture(nrChannels, is8bit);
}

Texture::Texture(char const* path)
{
	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		// generate texture
		glGenTextures(1, &ID);

		// initialize member variables
		size = glm::vec3(width, height, 0.f);
		// create texture info
		info = new TextureInfo(TextureType::twoDimensional);

		// determine the texture format
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		// bind texture and set its data
		glBindTexture(GL_TEXTURE_2D, ID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set texture properties
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// free texture data
		stbi_image_free(data);
	}
	else
	{
		ID = 0;
		size = glm::vec3(0.f);
		info = new TextureInfo(TextureType::faulty);

		std::cout << "Texture failed to load at path: " << path << std::endl;
	}
}

void Texture::bind(int binding)
{
	glBindImageTexture(binding, ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

Texture::~Texture()
{
	delete info;
	glDeleteTextures(1, &ID);
}

unsigned int Texture::generateGlTexture(uint8_t nrChannels, bool is8bit)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(info->glType, texture);

	glTexParameteri(info->glType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(info->glType, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(info->glType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(info->glType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLenum format{GL_RGBA32F};
	if (nrChannels == 1) {
		format = GL_RED;
	}
	else if (nrChannels == 3) {
		if (is8bit)
			format = GL_RGB8;
		else
			format = GL_RGB32F;
	}
	else if (nrChannels == 4) {
		if (is8bit)
			format = GL_RGBA8;
		else
			format = GL_RGBA32F;
	}
	else {
		std::cout << "ERROR::TEXTURE nrChannels invalidly set!" << std::endl;
	}

	switch (info->type)
	{
	case TextureType::oneDimensional:
		glTexImage1D(GL_TEXTURE_1D, 0, format, static_cast<GLsizei>(size.x), 0, GL_RGBA, GL_FLOAT, NULL);
		break;
	case TextureType::twoDimensional:
		glTexImage2D(GL_TEXTURE_2D, 0, format, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, GL_RGBA, GL_FLOAT, NULL);
		break;
	case TextureType::threeDimensional:
		glTexImage3D(GL_TEXTURE_3D, 0, format, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), static_cast<GLsizei>(size.z), 0, GL_RGBA, GL_FLOAT, NULL);
		break;
	default:
		std::cout << "ERROR::TEXTURE TextureType is invalidly set!" << std::endl;
		break;
	}

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, format);

	return texture;
}
