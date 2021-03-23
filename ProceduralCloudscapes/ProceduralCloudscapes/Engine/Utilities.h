#ifndef UTIL_H
#define UTIL_H

// NOTE: Always include GLAD before other header files that require OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <stb_image.h>

namespace util {
	template<typename T>
	static void eraseByValue(std::vector<T> vector, T value)
	{
		vector.erase(std::remove(vector.begin(), vector.end(), value), vector.end());
	}

    static unsigned int loadTexture(char const* path)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format{};
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
        }

        stbi_image_free(data);
        return textureID;
    }
}

#endif // !UTIL_H
