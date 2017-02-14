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
