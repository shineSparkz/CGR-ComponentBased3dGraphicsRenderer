#ifndef __SHADER_PROGRAM_H__
#define __SHADER_PROGRAM_H__

#include <string>
#include <vector>
#include <map>
#include "Shader.h"
#include "Uniform.h"

#define LOG_SHADER_ERRORS

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
};

template<typename T>
void ShaderProgram::SetUniformValue(const std::string& name, const T* v)
{
	//auto i = m_Uniforms.find(hash(name.c_str()));
	auto i = m_Uniforms.find(name);
	if (i != m_Uniforms.end())
		i->second->SetValue<T>(v);
#ifdef LOG_SHADER_ERRORS
	else
	{
		WRITE_LOG("Shader error: " + name, "error");
	}
#endif
}

#endif