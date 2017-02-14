#include "Font.h"
#include "LogFile.h"
#include "Renderer.h"
#include "Texture.h"
#include "OpenGlLayer.h"

Font::Font() :
	m_Characters(),
	m_Vao(0),
	m_Vbo(0)
{
}

Font::~Font()
{
}

bool Font::CreateFont(const std::string& font, int fontSize)
{
	FT_Library m_FTLibrary;
	if (FT_Init_FreeType(&m_FTLibrary))
	{
		WRITE_LOG("Could not Initialize freetype library.", "error");
		return false;
	}

	FT_Face m_FontFace;

	if (FT_New_Face(m_FTLibrary, font.c_str(), 0, &m_FontFace))
	{
		WRITE_LOG("Error Could not open font: cour.ttf", "error");
		return false;
	}

	FT_Set_Pixel_Sizes(m_FontFace, 0, fontSize);

	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(m_FontFace, c, FT_LOAD_RENDER))
		{
			WRITE_LOG("ERROR::FREETYTPE: Failed to load Glyph", "warning");
			continue;
		}

		GLuint texture;

		Texture::create_tex2D(
			&texture,
			GL_TEXTURE0,
			GL_RED,
			m_FontFace->glyph->bitmap.width,
			m_FontFace->glyph->bitmap.rows,
			GL_RED,
			GL_UNSIGNED_BYTE,
			m_FontFace->glyph->bitmap.buffer,
			GL_CLAMP_TO_EDGE,
			GL_CLAMP_TO_EDGE,
			GL_LINEAR,
			GL_LINEAR,
			false
		);

		// Now store character for later use
		Character character = {
			texture,
			m_FontFace->glyph->bitmap.width,
			m_FontFace->glyph->bitmap.rows,
			m_FontFace->glyph->bitmap_left,
			m_FontFace->glyph->bitmap_top,
			m_FontFace->glyph->advance.x
		};

		m_Characters.insert(std::pair<GLchar, Character>(c, character));
	}

	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &this->m_Vao);
	glGenBuffers(1, &this->m_Vbo);
	glBindVertexArray(this->m_Vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->m_Vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	FT_Done_Face(m_FontFace);
	FT_Done_FreeType(m_FTLibrary);

	return true;
}

void Font::Close()
{
	std::map<GLchar, Character>::iterator it;

	for (it = m_Characters.begin(); it != m_Characters.end(); ++it)
	{
		OpenGLLayer::clean_GL_texture(&it->second.textureID, 1);
	}

	m_Characters.clear();

	OpenGLLayer::clean_GL_buffer(&this->m_Vbo, 1);
	OpenGLLayer::clean_GL_vao(&this->m_Vao, 1);
}