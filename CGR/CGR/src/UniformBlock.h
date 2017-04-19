#ifndef  __UNIFORM_BLOCK_H__
#define  __UNIFORM_BLOCK_H__

#include "gl_headers.h"
#include "types.h"
#include <vector>
#include <string>
#include <map>

class UniformBlock
{
	struct BlockData
	{
		GLint size;
		GLint offset;
	};

public:
	UniformBlock();
	~UniformBlock();
	void Close();

	bool IsBound() const;
	bool ShouldUpdateGPU() const;
	std::vector<const char*> GetUniformNames();
	bool AddUniform(const std::string& uniformname);
	void SetValue(const std::string& uniformName, void* value);
	void Bind();
	void ClearBlock();

private:
	bool allocBlock(GLuint* shaderProg, const char* name);
	bool addBlockData(const std::string& uniformName, GLint size, GLint offset);

private:
	friend class						ShaderProgram;
	std::map<std::string, BlockData>	m_Uniforms;
	byte*								m_Buffer;
	GLint								m_BuffSize;
	GLuint								m_UboIndex;
	GLuint								m_UBO;
	bool								m_Bound;
	bool								m_ShouldUpdatGPU;
};


#endif // ! __UNIFORM_BLOCK_H__
