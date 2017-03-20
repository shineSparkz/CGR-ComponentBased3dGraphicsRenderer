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

ShaderProgram::ShaderProgram() :
	m_ShaderProgram(0),
	m_Uniforms()
{
}

ShaderProgram::~ShaderProgram()
{
}

bool ShaderProgram::CreateProgram(const std::vector<Shader>& shaders, const std::string& fragout_identifier, GLuint frag_loc)
{
	// Link shaders
	m_ShaderProgram = glCreateProgram();

	std::vector<Shader>::const_iterator it;
	bool has_fragment_shader = false;

	for (it = shaders.begin(); it != shaders.end(); ++it)
	{
		if (it->shader_type == GL_FRAGMENT_SHADER)
			has_fragment_shader = true;

		glAttachShader(m_ShaderProgram, (*it).shader);

		for (size_t j = 0; j < (*it).attributes.size(); ++j)
		{
			glBindAttribLocation(m_ShaderProgram, (*it).attributes[j].layout_location,
				(*it).attributes[j].name.c_str());

			if (OpenGLLayer::check_GL_error())
			{
				WRITE_LOG("Error: binding attrib location in shader", "error");
				return false;
			}
		}
	}

	// Bind Frag
	if (has_fragment_shader)
	{
		glBindFragDataLocation(m_ShaderProgram, frag_loc, fragout_identifier.c_str());
		if (OpenGLLayer::check_GL_error())
		{
			WRITE_LOG("Error: binding frag data location in shader", "error");
			return false;
		}
	}

	glLinkProgram(m_ShaderProgram);

	GLint link_status = 0;
	glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &link_status);

	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(m_ShaderProgram, string_length, NULL, log);
		std::stringstream logger;
		logger << log << std::endl;
		WRITE_LOG(logger.str(), "error");
		return false;
	}

	//------------------------------------------------------
	// Populate m_Uniforms
	//------------------------------------------------------
	int total = -1;
	glGetProgramiv(m_ShaderProgram, GL_ACTIVE_UNIFORMS, &total);

	for (int i = 0; i < total; ++i)
	{
		int name_len = -1, num = -1;
		GLenum type = GL_ZERO;
		char name[100];

		glGetActiveUniform(
			m_ShaderProgram,
			GLuint(i),
			sizeof(name) - 1,
			&name_len,
			&num,
			&type,
			name);

		name[name_len] = 0;
		GLuint location = glGetUniformLocation(m_ShaderProgram, name);

		Uniform* ufm = new Uniform(location, (UniformTypes)type);
		//m_Uniforms[hash(name)] = ufm;
		m_Uniforms[name] = ufm;
	}

	return true;
}

Uniform* ShaderProgram::GetUniformByName(const std::string& name)
{
	//auto i = m_Uniforms.find(hash(name.c_str()));
	auto i = m_Uniforms.find(name);
	return ((i != m_Uniforms.end()) ? i->second : nullptr);
}

void ShaderProgram::Close()
{
	for (auto i = m_Uniforms.begin(); i != m_Uniforms.end(); ++i)
	{
		SAFE_DELETE(i->second);
	}

	m_Uniforms.clear();

	// Clean GL stuff
	OpenGLLayer::clean_GL_program(&m_ShaderProgram);
}

void ShaderProgram::Use()
{
	glUseProgram(m_ShaderProgram);
}



Uniform::Uniform(int location, UniformTypes utype) :
	m_UType(utype),
	m_Location(location),
	m_CurrentValue()
{
}

void Uniform::SendGPU()
{
	switch (m_UType)
	{
	case U_FLOAT:
		glUniform1f(m_Location, *(float*)m_CurrentValue);
		break;
	case U_INT:
		glUniform1i(m_Location, *(int*)m_CurrentValue);
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
		glUniform2fv(m_Location, 1, glm::value_ptr(*(Vec2*)m_CurrentValue));
		break;
	case U_VEC3:
		glUniform3fv(m_Location, 1, glm::value_ptr(*(Vec3*)m_CurrentValue));
		break;
	case U_VEC4:
		glUniform4fv(m_Location, 1, glm::value_ptr(*(Vec4*)m_CurrentValue));
		break;
	case U_MAT2:
		break;
	case U_MAT3:
		break;
	case U_MAT4:
		glUniformMatrix4fv(m_Location, 1, GL_FALSE, glm::value_ptr(*(Mat4*)m_CurrentValue));
		break;
	case U_SAMPLER:
		glUniform1i(m_Location, *(int*)m_CurrentValue);
		break;
	case U_CUBE_SAMPLER:
		glUniform1i(m_Location, *(int*)m_CurrentValue);
		break;
	default:
		break;
	}
}