#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "gl_headers.h"
#include <string>

class Image;

class Texture
{
public:
	Texture(const std::string& name, GLenum target, int activeTexture);
	~Texture();

	static bool createTex3D(GLuint* texture, Image* images[6]);

	static bool create_tex2D(GLuint* textureOut, int active, GLint internalformat,
		GLsizei width, GLsizei height, GLenum format, GLenum type, const void* data,
		GLint wrapS, GLint wrapT, GLint minFilter, GLint magFilter, bool mips);

	bool Create(Image* img);
	bool Create(Image* images[6]);
	void Bind();

	const std::string& Name() const
	{
		return name;
	}

private:
	std::string name;
	int m_ActiveTexture;
	GLenum m_Target;
	GLuint m_TexturePtr;
};

#endif