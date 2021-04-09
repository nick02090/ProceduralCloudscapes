#include "Texture.h"

#include <iostream>

Texture::Texture(TextureType _type, size_t _size, uint8_t nrChannels)
{
	// initialize member variables
	size = _size;
	// create texture info
	info = new TextureInfo(_type);
	// create GL texture
	ID = generateGlTexture(nrChannels);
}

Texture::~Texture()
{
	delete info;
	glDeleteTextures(1, &ID);
}

unsigned int Texture::generateGlTexture(uint8_t nrChannels)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(info->glType, texture);

	glTexParameteri(info->glType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(info->glType, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(info->glType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(info->glType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLenum format{};
	if (nrChannels == 1)
		format = GL_RED;
	else if (nrChannels == 3)
		format = GL_RGB;
	else if (nrChannels == 4)
		format = GL_RGBA;

	switch (info->type)
	{
	case TextureType::oneDimensional:
		glTexImage1D(GL_TEXTURE_1D, 0, format, static_cast<GLsizei>(size), 0, format, GL_FLOAT, NULL);
		break;
	case TextureType::twoDimensional:
		glTexImage2D(GL_TEXTURE_2D, 0, format, static_cast<GLsizei>(size), static_cast<GLsizei>(size), 0, format, GL_FLOAT, NULL);
		break;
	case TextureType::threeDimensional:
		glTexImage3D(GL_TEXTURE_3D, 0, format, static_cast<GLsizei>(size), static_cast<GLsizei>(size), static_cast<GLsizei>(size), 0, format, GL_FLOAT, NULL);
		break;
	default:
		std::cout << "ERROR::TEXTURE TextureType is invalidly set!" << std::endl;
		break;
	}

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, format);

	return texture;
}
