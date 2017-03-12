#ifndef __BILLBOARD_LIST_H__
#define __BILLBOARD_LIST_H_

#include <vector>
#include "gl_headers.h"
#include "types.h"

class BillboardTechnique;
class Renderer;

class BillboardList
{
public:
	BillboardList();
	~BillboardList();

	bool Init(BillboardTechnique* mat, unsigned textureIndex, float setScale, unsigned numX, unsigned numY, float spacing, float startOffset, float yPos);
	bool InitWithPositions(BillboardTechnique* mat, unsigned texture, float setScale, const std::vector<Vec3>& positions);

	void Render(Renderer* renderer, const Mat4& viewProj, const Vec3& camPos, const Vec3& camRight);

private:
	BillboardTechnique* m_Material;
	unsigned m_TextureIndex;
	unsigned m_NumInstances;
	GLuint m_VBO;
	GLuint m_VAO;
	float m_BillboardScale;
};

#endif