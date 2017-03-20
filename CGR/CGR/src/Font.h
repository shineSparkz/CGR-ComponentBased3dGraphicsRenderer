#ifndef __FONT_H__
#define __FONT_H__

#include <map>
#include <string>
#include "gl_headers.h"

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character
{
	GLuint textureID;
	int sizeX;
	int sizeY;
	int bearingX;
	int bearingY;
	signed long advance;
};

class Font
{
public:
	Font();
	~Font();

	bool CreateFont(const std::string& font, int fontSize);
	void Close();

private:
	friend class Renderer;
	std::map<GLchar, Character> m_Characters;
	GLuint m_Vao;
	GLuint m_Vbo;
};

#endif