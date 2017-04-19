#ifndef __SHADOW_FRAME_BUFFER_H__
#define __SHADOW_FRAME_BUFFER_H__

#include "gl_headers.h"
#include "types.h"

class ShadowFrameBuffer
{
public:
	ShadowFrameBuffer();
	~ShadowFrameBuffer();

	bool Init(int windowWidth, int windowHeight);
	void BindForWriting();
	void BindForReading(GLenum texUnit);

private:
	GLuint m_FrameBufferObj;
	GLuint m_ShadowMap;
};

#endif