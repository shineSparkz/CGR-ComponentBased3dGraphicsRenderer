#include "Uniform.h"
#include <glm/gtc/type_ptr.hpp>
#include "types.h"

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