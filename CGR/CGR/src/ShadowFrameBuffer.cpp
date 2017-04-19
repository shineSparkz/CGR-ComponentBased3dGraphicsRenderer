#include "ShadowFrameBuffer.h"
#include "LogFile.h"

ShadowFrameBuffer::ShadowFrameBuffer() :
	m_FrameBufferObj(0),
	m_ShadowMap(0)
{
}

ShadowFrameBuffer::~ShadowFrameBuffer()
{
}

bool ShadowFrameBuffer::Init(int windowWidth, int windowHeight)
{
	// Gen Frame buffer
	glGenFramebuffers(1, &m_FrameBufferObj);

	// Gen and create shadow texture
	glGenTextures(1, &m_ShadowMap);
	glBindTexture(GL_TEXTURE_2D, m_ShadowMap);

	// Target, level, internalFormat, width, height, border, format, type, pixels
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT,NULL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferObj);

	// Attach texture object to framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		WRITE_LOG("Shadow map frame buffer object", "error");
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void ShadowFrameBuffer::BindForWriting()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBufferObj);
}

void ShadowFrameBuffer::BindForReading(GLenum texUnit)
{
	glActiveTexture(texUnit);
	glBindTexture(GL_TEXTURE_2D, m_ShadowMap);
}