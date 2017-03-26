#include "OpenGlLayer.h"

#include "LogFile.h"

void OpenGLLayer::enable_GL_state(GLenum state)
{
	glEnable(state);
}

void OpenGLLayer::disable_GL_state(GLenum state)
{
	glDisable(state);
}

bool OpenGLLayer::check_GL_error()
{
	GLenum err = glGetError();

	if (err != GL_NO_ERROR)
	{
		const char* gl_err_str = (const char*)glewGetErrorString(err);
		std::string log_str = "GL Error: ";
		log_str += gl_err_str;
		WRITE_LOG(log_str, "error");
		return true;
	}

	return false;
}

void OpenGLLayer::clean_GL_texture(GLuint* texID, int count)
{
	if (texID)
	{
		if (glIsTexture(*texID))
		{
			glDeleteTextures(count, texID);
			OpenGLLayer::check_GL_error();
		}
	}
}

void OpenGLLayer::clean_GL_buffer(GLuint* buff, int count)
{
	if (buff)
	{
		if (glIsBuffer(*buff))
		{
			glDeleteBuffers(count, buff);
			OpenGLLayer::check_GL_error();
		}
	}
}

void OpenGLLayer::clean_GL_vao(GLuint* buff, int count)
{
	if (buff)
	{
		if (glIsVertexArray(*buff))
		{
			glDeleteVertexArrays(count, buff);
			OpenGLLayer::check_GL_error();
		}
	}
}

void OpenGLLayer::clean_GL_program(GLuint* buff)
{
	if (buff)
	{
		if (glIsProgram(*buff))
		{
			glDeleteProgram(*buff);
			OpenGLLayer::check_GL_error();
		}
	}
}

void OpenGLLayer::clean_GL_shader(GLuint* shader)
{
	if (glIsShader(*shader))
	{
		glDeleteShader(*shader);
		OpenGLLayer::check_GL_error();
	}
}

void OpenGLLayer::clean_GL_fbo(GLuint* fbo, int count)
{
	if (glIsFramebuffer(*fbo))
	{
		glDeleteFramebuffers(count, fbo);
	}
}

size_t OpenGLLayer::glTypeSize(GLenum type)
{
	size_t size = 0;
#define CASE(Enum, Count, Type) \
case Enum: size = Count * sizeof(Type); break
	switch (type)
	{
		CASE(GL_FLOAT, 1, GLfloat);
		CASE(GL_FLOAT_VEC2, 2, GLfloat);
		CASE(GL_FLOAT_VEC3, 3, GLfloat);
		CASE(GL_FLOAT_VEC4, 4, GLfloat);
		CASE(GL_INT, 1, GLint);
		CASE(GL_INT_VEC2, 2, GLint);
		CASE(GL_INT_VEC3, 3, GLint);
		CASE(GL_INT_VEC4, 4, GLint);
		CASE(GL_UNSIGNED_INT, 1, GLuint);
		CASE(GL_UNSIGNED_INT_VEC2, 2, GLuint);
		CASE(GL_UNSIGNED_INT_VEC3, 3, GLuint);
		CASE(GL_UNSIGNED_INT_VEC4, 4, GLuint);
		CASE(GL_BOOL, 1, GLboolean);
		CASE(GL_BOOL_VEC2, 2, GLboolean);
		CASE(GL_BOOL_VEC3, 3, GLboolean);
		CASE(GL_BOOL_VEC4, 4, GLboolean);
		CASE(GL_FLOAT_MAT2, 4, GLfloat);
		CASE(GL_FLOAT_MAT2x3, 6, GLfloat);
		CASE(GL_FLOAT_MAT2x4, 8, GLfloat);
		CASE(GL_FLOAT_MAT3, 9, GLfloat);
		CASE(GL_FLOAT_MAT3x2, 6, GLfloat);
		CASE(GL_FLOAT_MAT3x4, 12, GLfloat);
		CASE(GL_FLOAT_MAT4, 16, GLfloat);
		CASE(GL_FLOAT_MAT4x2, 8, GLfloat);
		CASE(GL_FLOAT_MAT4x3, 12, GLfloat);
#undef CASE
	default:
		WRITE_LOG("UNKNOWN GL TYPE", "error");
		break;
	}

	return size;
}