#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "gl_headers.h"

class Image;

class Texture
{
public:
	Texture(GLenum target, int activeTexture);
	~Texture();

	static bool create_tex2D(GLuint* textureOut, int active, GLint internalformat,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* data,
		GLint wrapS, GLint wrapT, GLint minFilter, GLint magFilter, bool mips);

	bool Create(Image* img);
	void Bind();

private:
	int m_ActiveTexture;
	GLenum m_Target;
	GLuint m_TexturePtr;
};

#endif