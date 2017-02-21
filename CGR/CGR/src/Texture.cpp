#include "Texture.h"

#include "OpenGlLayer.h"
#include "Image.h"
#include "LogFile.h"

bool Texture::createTex3D(GLuint* texture, Image* images[6])
{
	glGenTextures(1, texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, *texture);

	GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_BGR, GL_BGRA };

	for (size_t i = 0; i < 6; ++i)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGBA,
			images[i]->Width(),
			images[i]->Height(),
			0,
			pixel_formats[images[i]->NumBytes()],
			GL_UNSIGNED_BYTE,
			images[i]->Data()
			);

		OpenGLLayer::check_GL_error();
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return true;
}

bool Texture::create_tex2D(GLuint* textureOut, int active, GLint internalformat,
	GLsizei width, GLsizei height, GLenum format, GLenum type, const void* data,
	GLint wrapS, GLint wrapT, GLint minFilter, GLint magFilter, bool mips)
{
	// Checks
	if (magFilter != GL_NEAREST && magFilter != GL_LINEAR)
	{
		WRITE_LOG("Warning: mag filter on texture bind not set to nearest or linear, have defaulted to GL_NEAREST", "warning");
		magFilter = GL_NEAREST;
	}

	if ((minFilter == GL_NEAREST || minFilter == GL_LINEAR) && mips == true)
	{
		WRITE_LOG("Warning: min filter must have a mipmap type if you want to allow mipmaps, have tuened mips off", "warning");
		mips = false;
	}

	// Now create Texture
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, textureOut);
	glBindTexture(GL_TEXTURE_2D, *textureOut);
	glActiveTexture(active);

	// Set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internalformat,
		width,
		height,
		0,
		format,
		type,
		data
		);

	OpenGLLayer::check_GL_error();

	if (mips)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		OpenGLLayer::check_GL_error();
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}


Texture::Texture(GLenum target, int active) :
	m_Target(target),
	m_ActiveTexture(active),
	m_TexturePtr(0)
{
}

Texture::~Texture()
{
	OpenGLLayer::clean_GL_texture(&m_TexturePtr, 1);
}

bool Texture::Create(Image* images[6])
{
	if (!createTex3D(&m_TexturePtr, images))
	{
		WRITE_LOG("Failed to create gl cubemap", "error");
		return false;
	}

	if (!glIsTexture(m_TexturePtr))
	{
		WRITE_LOG("fail texture", "warning");
		return false;
	}

	return true;
}

bool Texture::Create(Image* img)
{
	GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_BGR, GL_BGRA };

	// If tex2D
	if (!create_tex2D(
		&m_TexturePtr,
		m_ActiveTexture,
		GL_RGBA,
		img->Width(),
		img->Height(),
		pixel_formats[img->NumBytes()],
		GL_UNSIGNED_BYTE,
		img->Data(),
		// Pass these in, but have defaults
		GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true))
	{
		WRITE_LOG("Failed to create gl texture", "error");
		return false;
	}

	if (!glIsTexture(m_TexturePtr))
	{
		WRITE_LOG("fail texture", "warning");
		return false;
	}

	return true;
}

void Texture::Bind()
{
	glActiveTexture(m_ActiveTexture);
	glBindTexture(m_Target, m_TexturePtr);
}