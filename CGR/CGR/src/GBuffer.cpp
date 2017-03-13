#include "GBuffer.h"
#include "Screen.h"
#include "Texture.h"
#include "Renderer.h"
#include "OpenGlLayer.h"
#include "LogFile.h"

const GLenum DRAW_BUFFERS[] = 
{
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2 
};

GBuffer::GBuffer() :
	m_FBO(0),
	m_DepthTexture(0),
	m_FinalTexture(0)
{
}

GBuffer::~GBuffer()
{
	OpenGLLayer::clean_GL_buffer(&m_FBO, 1);
	OpenGLLayer::clean_GL_texture(&m_DepthTexture, 1);
	OpenGLLayer::clean_GL_texture(&m_FinalTexture, 1);
	OpenGLLayer::clean_GL_texture(m_Textures, ARRAY_SIZE_IN_ELEMENTS(m_Textures));
}

bool GBuffer::Init()
{
	int width =  Screen::Instance()->FrameBufferWidth();
	int height = Screen::Instance()->FrameBufferHeight();

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

	// Create GBuffer Textures
	glGenTextures(ARRAY_SIZE_IN_ELEMENTS(m_Textures), m_Textures);
	glGenTextures(1, &m_DepthTexture);
	glGenTextures(1, &m_FinalTexture);

	for (unsigned i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_Textures); ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_Textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_Textures[i], 0);
	}

	// Depth
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);

	// Final
	glBindTexture(GL_TEXTURE_2D, m_FinalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_FinalTexture, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		WRITE_LOG("GBuffer error", "error");
		return false;
	}

	// Restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return true;
}

void GBuffer::StartFrame()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT4);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GBuffer::BindForGeomPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
	glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DRAW_BUFFERS), DRAW_BUFFERS);
}

void GBuffer::BindForStencilPass()
{
	// Must disable the draw buffers 
	glDrawBuffer(GL_NONE);
}

void GBuffer::BindForLightPass()
{
	glDrawBuffer(GL_COLOR_ATTACHMENT4);

	for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_Textures); ++i) 
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_Textures[TexTypes::Position + i]);
	}
}

void GBuffer::BindForFinalPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
	glReadBuffer(GL_COLOR_ATTACHMENT4);
}



