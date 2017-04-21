#ifndef __BILLBOARD_LIST_H__
#define __BILLBOARD_LIST_H_

#include <vector>
#include "gl_headers.h"
#include "types.h"
#include "Vertex.h"

class ShaderProgram;
class Renderer;

class BillboardList
{
public:
	BillboardList();
	~BillboardList();

	bool	Init				(size_t shader, size_t textureIndex, float setScale, size_t numX, size_t numY, float spacing, float startOffset, float yPos);
	bool	InitWithPositions	(size_t shader, size_t texture, float setScale, const std::vector<Vec3>& positions);
	void	SetScale			(float scale);
	float	GetScale			() const;

private:
	friend class Renderer;
	size_t			m_ShaderIndex;
	size_t			m_TextureIndex;
	size_t			m_NumInstances;
	GLuint			m_VBO;
	GLuint			m_VAO;
	float			m_BillboardScale;
};

#endif