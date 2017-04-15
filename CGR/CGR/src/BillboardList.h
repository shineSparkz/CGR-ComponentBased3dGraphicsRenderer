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

	bool Init				(ShaderProgram* mat, size_t textureIndex, float setScale, size_t numX, size_t numY, float spacing, float startOffset, float yPos);
	bool InitWithPositions	(ShaderProgram* mat, size_t texture, float setScale, const std::vector<Vec3>& positions);
	bool SetShaderProgram	(ShaderProgram* shader);

private:
	friend class Renderer;
	ShaderProgram*	m_Material;
	size_t			m_TextureIndex;
	size_t			m_NumInstances;
	GLuint			m_VBO;
	GLuint			m_VAO;
	float			m_BillboardScale;
};

#endif