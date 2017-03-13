#ifndef __G_BUFFER_H__
#define __G_BUFFER_H__

#include <vector>
#include "gl_headers.h"

#define GBUFFER_POSITION_TEXTURE_UNIT 0
#define GBUFFER_DIFFUSE_TEXTURE_UNIT  1
#define GBUFFER_NORMAL_TEXTURE_UNIT   2

class GBuffer
{
public:
	enum TexTypes
	{
		Position,
		Diffuse,
		Normal,
		Texcoord,
		NUM_GBUF_TEX_TYPES
	};

	GBuffer();
	~GBuffer();

	// TODO : Make a base class for buffer object
	bool Init();

	void StartFrame();
	void BindForGeomPass();
	void BindForStencilPass();
	void BindForLightPass();
	void BindForFinalPass();

private:
	GLuint m_FBO;
	GLuint m_DepthTexture;
	GLuint m_FinalTexture;
	GLuint m_Textures[NUM_GBUF_TEX_TYPES];
};

#endif