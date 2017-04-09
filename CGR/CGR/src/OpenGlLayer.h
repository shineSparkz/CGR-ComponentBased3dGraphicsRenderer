#ifndef __OPEN_GL_LAYER_H__
#define __OPEN_GL_LAYER_H__

#include "gl_headers.h"

class OpenGLLayer
{
public:
	static bool		check_GL_error		();
	static void		enable_GL_state		(GLenum state);
	static void		disable_GL_state	(GLenum state);
	static void		clean_GL_texture	(GLuint* texID, int count);
	static void		clean_GL_buffer		(GLuint* buff, int count);
	static void		clean_GL_vao		(GLuint* buff, int count);
	static void		clean_GL_program	(GLuint* buff);
	static void		clean_GL_shader		(GLuint* shader);
	static void		clean_GL_fbo		(GLuint* fbo, int count);
	static size_t	glTypeSize			(GLenum type);
};

#endif
