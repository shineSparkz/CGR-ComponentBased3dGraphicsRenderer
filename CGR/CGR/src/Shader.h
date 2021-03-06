#ifndef __SHADERS_H__
#define __SHADERS_H__

#include "gl_headers.h"
#include "types.h"
#include "LogFile.h"
#include "Singleton.h"

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
	Shader(GLenum type);
	~Shader();

	void Close();
	bool LoadShader(const char* src);

	void AddAttributes(const std::vector<ShaderAttrib>& attribs);
	void AddAttribute(const ShaderAttrib& attrib);

private:
	friend class				ShaderProgram;
	std::vector<ShaderAttrib>	m_Attributes;
	std::string					m_SourceFile;
	GLenum						m_ShaderType;
	GLuint						m_Shader;
};

#endif