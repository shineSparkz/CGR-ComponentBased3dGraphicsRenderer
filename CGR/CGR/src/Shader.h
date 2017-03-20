#ifndef __SHADERS_H__
#define __SHADERS_H__

#include "gl_headers.h"
#include "types.h"

#include <string>
#include <vector>
#include <map>
#include <functional>

dword hash(const char *str);

struct ShaderAttrib
{
	GLuint layout_location;
	std::string name;
};

class Shader
{
public:
	Shader();
	Shader(GLenum type);
	~Shader();

	void Close();
	bool LoadShader(const char* src);
	void AddAttributes(const std::vector<ShaderAttrib>& attribs);
	void AddAttribute(const ShaderAttrib& attrib);

private:
	friend class RenderTechnique;
	friend class ShaderProgram;
	std::vector<ShaderAttrib> attributes;
	GLenum shader_type;
	GLuint shader;
};


// -----------------
enum UniformTypes
{
	U_FLOAT = GL_FLOAT,
	U_INT = GL_INT,
	U_INT2 = GL_INT_VEC2,
	U_INT3 = GL_INT_VEC3,
	U_INT4 = GL_INT_VEC4,
	U_BOOL = GL_BOOL,
	U_BOOL2 = GL_BOOL_VEC2,
	U_BOOL3 = GL_BOOL_VEC3,
	U_BOOL4 = GL_BOOL_VEC4,
	U_VEC2 = GL_FLOAT_VEC2,
	U_VEC3 = GL_FLOAT_VEC3,
	U_VEC4 = GL_FLOAT_VEC4,
	U_MAT2 = GL_FLOAT_MAT2,
	U_MAT3 = GL_FLOAT_MAT3,
	U_MAT4 = GL_FLOAT_MAT4,
	U_SAMPLER = GL_SAMPLER_2D,
	U_CUBE_SAMPLER = GL_SAMPLER_CUBE
};

class Uniform
{
public:
	Uniform(int location, UniformTypes uniformType);

	template<typename T>
	void SetValue(const T* v);

	void* GetValue() const
	{
		return m_CurrentValue;
	}

private:
	virtual void SendGPU();

private:
	friend class ShaderProgram;
	UniformTypes m_UType;
	GLint m_Location;
	void* m_CurrentValue;
};

template<typename T>
void Uniform::SetValue(const T* v)
{
	m_CurrentValue = (void*)v;
	SendGPU();
}

class ShaderProgram
{
public:
	ShaderProgram();
	virtual ~ShaderProgram();

	template<typename T>
	void SetUniformValue(const std::string& name, const T* v);

	void Use();

	void Close();
	bool CreateProgram(const std::vector<Shader>& shaders, const std::string& fragout_identifier, GLuint frag_loc);
	Uniform* GetUniformByName(const std::string& name);

private:
	friend class Renderer;
	GLuint m_ShaderProgram;
	std::map<std::string, Uniform*> m_Uniforms;
	//std::map<dword, Uniform*> m_Uniforms;
};

template<typename T>
void ShaderProgram::SetUniformValue(const std::string& name, const T* v)
{
	//auto i = m_Uniforms.find(hash(name.c_str()));
	auto i = m_Uniforms.find(name);
	if (i != m_Uniforms.end())
		i->second->SetValue<T>(v);
}

#endif