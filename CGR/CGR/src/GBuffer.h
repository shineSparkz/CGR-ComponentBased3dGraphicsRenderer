#ifndef __G_BUFFER_H__
#define __G_BUFFER_H__

#include <vector>
#include "gl_headers.h"

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

	bool Init();
	void Clean();

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