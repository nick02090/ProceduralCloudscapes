#ifndef TEXTURE_H
#define TEXTURE_H

// include glad to get all the required OpenGL headers
#include <glad/glad.h>

#include <string>

enum class TextureType {
	oneDimensional = 0,
	twoDimensional = 1,
	threeDimensional = 2
};

struct TextureInfo {
	TextureInfo(TextureType _type) : type(_type) {
		switch (type)
		{
		case TextureType::oneDimensional:
			name = "Tex_1D";
			glType = GL_TEXTURE_1D;
			break;
		case TextureType::twoDimensional:
			name = "Tex_2D";
			glType = GL_TEXTURE_2D;
			break;
		case TextureType::threeDimensional:
			name = "Tex_3D";
			glType = GL_TEXTURE_3D;
			break;
		default:
			name = "";
			glType = -1;
			break;
		}
	}
	TextureType type;
	unsigned int glType;
	std::string name;
};

class Texture {
public:
	// texture ID
	unsigned int ID;

	Texture(TextureType _type, size_t _size, uint8_t nrChannels);
	~Texture();

private:
	unsigned int generateGlTexture(uint8_t nrChannels);

	TextureInfo* info;
	size_t size;
};

#endif // !TEXTURE_H
