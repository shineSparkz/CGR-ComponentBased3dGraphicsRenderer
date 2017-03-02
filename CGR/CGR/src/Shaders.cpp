#include "Shader.h"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "utils.h"
#include "LogFile.h"
#include "Renderer.h"
#include "OpenGlLayer.h"
#include "TextFile.h"

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

