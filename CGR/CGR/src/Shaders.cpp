#include "Shader.h"

#include <iostream>

#include "utils.h"
#include "Renderer.h"
#include "OpenGlLayer.h"
#include "TextFile.h"

#include "UniformBlockManager.h"
#include "UniformBlock.h"

dword hash(const char *str)
{
	dword hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

Shader::Shader() :
	m_Attributes(),
	m_SourceFile(""),
	m_ShaderType(-1),
	m_Shader(0)
{
}

Shader::Shader(GLenum type) :
	m_Attributes(),
	m_SourceFile(""),
	m_ShaderType(type),
	m_Shader(0)
{
}

Shader::~Shader()
{
}

bool Shader::LoadShader(const char* srcPath)
{
	TextFile tf;
	std::string shadersrc = tf.LoadFileIntoStr(srcPath);
	if (shadersrc == "")
	{
		return false;
	}

	// Need to cast to a c string for open GL
	const char* src = shadersrc.c_str();

	// Ask openGL to create a shader of passed in type
	m_Shader = glCreateShader(m_ShaderType);

	// Ask OpenGL to attempt shader compilation
	GLint compile_status = 0;
	glShaderSource(m_Shader, 1, (const GLchar **)&src, NULL);
	glCompileShader(m_Shader);
	glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status != GL_TRUE)
	{
		// Log what went wrong in shader src
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(m_Shader, string_length, NULL, log);
		std::stringstream logger;
		logger << log << std::endl;
		WRITE_LOG(logger.str(), "error");
		return false;
	}

	m_SourceFile = srcPath;
	return true;
}

void Shader::AddAttribute(const ShaderAttrib& attrib)
{
	m_Attributes.push_back(attrib);
}

void Shader::AddAttributes(const std::vector<ShaderAttrib>& attribs)
{
	for each (ShaderAttrib sa in attribs)
	{
		this->m_Attributes.push_back(sa);
	}
}

void Shader::Close()
{
	OpenGLLayer::clean_GL_shader(&m_Shader);
}









