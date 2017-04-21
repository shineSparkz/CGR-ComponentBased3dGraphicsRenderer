#ifndef __ANIM_MESH_H__
#define __ANIM_MESH_H__

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string>
#include "anim_types.h"
#include "gl_headers.h"

class Texture;
class ResourceManager;
class ShaderProgram;
class BaseCamera;

class AnimMesh
{
public:
	AnimMesh();
	~AnimMesh();

	void Close();
	bool Load(const char* sFilename, ResourceManager* resMan, unsigned materialSet);
	static anim_t	animlist[21];

private:
	friend class						Renderer;
	std::vector<AnimData>				m_AnimData;		
	std::vector<int>					m_RenderModes;
	std::vector<int>					m_NumRenderVertices;
	uint32								m_VAO;				
};

#endif