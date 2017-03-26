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
	Shader();
	Shader(GLenum type);
	~Shader();

	void Close();
	//bool LoadShader(const std::string& src);
	bool LoadShader(const char* src);
	bool LoadShader(std::vector<std::string> srcs);

	void AddAttributes(const std::vector<ShaderAttrib>& attribs);
	void AddAttribute(const ShaderAttrib& attrib);

private:
	friend class RenderTechnique;
	friend class ShaderProgram;
	std::vector<ShaderAttrib> attributes;
	GLenum shader_type;
	GLuint shader;
};

#endif