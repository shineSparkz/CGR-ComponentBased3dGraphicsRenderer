#include "Shader.h"

#include <iostream>

#include "utils.h"
#include "Renderer.h"
#include "OpenGlLayer.h"
#include "TextFile.h"

#include "UniformBlockManager.h"
#include "UniformBlock.h"

Shader::Shader() :
	attributes(),
	shader_type(-1),
	shader(-1)
{
}

Shader::Shader(GLenum type) :
	attributes(),
	shader_type(type),
	shader(-1)
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
	this->shader = glCreateShader(this->shader_type);

	// Ask OpenGL to attempt shader compilation
	GLint compile_status = 0;
	glShaderSource(shader, 1, (const GLchar **)&src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status != GL_TRUE)
	{
		// Log what went wrong in shader src
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(shader, string_length, NULL, log);
		std::stringstream logger;
		logger << log << std::endl;
		WRITE_LOG(logger.str(), "error");
		return false;
	}

	return true;
}

bool Shader::LoadShader(std::vector<std::string> srcs)
{
	std::vector<std::string> fromFiles;
	for (int i = 0; i < srcs.size(); ++i)
	{
		TextFile tf;
		std::string shadersrc = tf.LoadFileIntoStr(srcs[i]);
		if (shadersrc == "")
		{
			return false;
		}

		fromFiles.push_back(shadersrc);
	}


	// Need to cast to a c string for open GL
	std::vector<const char*> glSrc;
	for (int i = 0; i < fromFiles.size(); ++i)
	{
		glSrc.push_back(fromFiles[i].c_str());
	}
	//const char* src = shadersrc.c_str();

	// Ask openGL to create a shader of passed in type
	this->shader = glCreateShader(this->shader_type);

	// Ask OpenGL to attempt shader compilation
	GLint compile_status = 0;
	glShaderSource(shader, (GLsizei)glSrc.size(), (const GLchar**)glSrc.data(), NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

	if (compile_status != GL_TRUE)
	{
		// Log what went wrong in shader src
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(shader, string_length, NULL, log);
		std::stringstream logger;
		logger << log << std::endl;
		WRITE_LOG(logger.str(), "error");
		return false;
	}

	return true;
}

void Shader::AddAttribute(const ShaderAttrib& attrib)
{
	this->attributes.push_back(attrib);
}

void Shader::AddAttributes(const std::vector<ShaderAttrib>& attribs)
{
	for each (ShaderAttrib sa in attribs)
	{
		this->attributes.push_back(sa);
	}
}

void Shader::Close()
{
	OpenGLLayer::clean_GL_shader(&this->shader);
}


/*
void setGluniform(UniformTypes ut)
{
	switch (ut)
	{
	case U_FLOAT:
		glUniform1f(
		break;
	case U_INT:
		break;
	case U_INT2:
		break;
	case U_INT3:
		break;
	case U_INT4:
		break;
	case U_BOOL:
		break;
	case U_BOOL2:
		break;
	case U_BOOL3:
		break;
	case U_BOOL4:
		break;
	case U_VEC2:
		break;
	case U_VEC3:
		break;
	case U_VEC4:
		break;
	case U_MAT2:
		break;
	case U_MAT3:
		break;
	case U_MAT4:
		break;
	case U_SAMPLER:
		break;
	case U_CUBE_SAMPLER:
		break;
	default:
		break;
	}
}
*/

dword hash(const char *str)
{
	dword hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}









