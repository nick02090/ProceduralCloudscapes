#ifndef FRAME_BUFFER_OBJECT_H
#define FRAME_BUFFER_OBJECT_H

// include glad to get all the required OpenGL headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

class Texture;

class FrameBufferObject {
public:
	// FBO ID
	unsigned int FBO;

	FrameBufferObject();
	~FrameBufferObject();

	void bind() const;
	void clear() const;
	// binds back to default framebuffer
	static void unbind();

	// determines whether the FBO is complete
	bool checkStatus();

	void attachColorTexture(unsigned int width, unsigned int height);
	void attachDepthTexture(unsigned int width, unsigned int height);

	Texture* getColorTexture(size_t texIndex) const { return colorTextures.at(texIndex); }
	inline unsigned int getDepthTextureID() const { return depthTexture; }
private:
	unsigned int depthTexture{ 0 };
	std::vector<Texture*> colorTextures;

	// determines the suitable GLenum for the current color texture
	GLenum getColorAttachmentNumber() const;
};

#endif // !FRAME_BUFFER_OBJECT_H
