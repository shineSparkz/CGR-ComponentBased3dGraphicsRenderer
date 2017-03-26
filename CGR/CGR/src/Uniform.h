#ifndef __UNIFORM_H__
#define __UNIFORM_H__

#include "gl_headers.h"

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


#endif