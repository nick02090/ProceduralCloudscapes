#include "FrameBufferObject.h"

#include "Texture.h"

FrameBufferObject::FrameBufferObject()
{
	glGenFramebuffers(1, &FBO);
}

FrameBufferObject::~FrameBufferObject()
{
	// delete the frame buffer
	glDeleteFramebuffers(1, &FBO);
	// delete color attachments
	for (auto colorTex : colorTextures)
	{
		glDeleteTextures(1, &colorTex->ID);
	}
}

void FrameBufferObject::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void FrameBufferObject::clear() const
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBufferObject::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FrameBufferObject::checkStatus()
{
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return false;
	}
	return true;
}

void FrameBufferObject::attachColorTexture(unsigned int width, unsigned int height)
{
	// first bind the FBO
	bind();

	Texture* colorTex = new Texture(TextureType::twoDimensional, glm::vec3(width, height, 0.0), 3, false);

	glFramebufferTexture2D(GL_FRAMEBUFFER, getColorAttachmentNumber(), colorTex->getGLType(), colorTex->ID, 0);

	// unbind the FBO since the configuration is done
	unbind();

	// add the attachment to the list
	colorTextures.push_back(colorTex);
}

void FrameBufferObject::attachDepthTexture(unsigned int width, unsigned int height)
{
	// first bind the FBO
	bind();

	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	// unbind the FBO since the configuration is done
	unbind();
}

GLenum FrameBufferObject::getColorAttachmentNumber() const
{
	return static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + colorTextures.size());
}
