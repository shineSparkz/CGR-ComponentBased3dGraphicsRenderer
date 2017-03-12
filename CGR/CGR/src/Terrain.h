#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include <string>
#include <vector>
#include "types.h"
#include "gl_headers.h"

class TerrainTechnique;
class Renderer;
class Camera;
class DirectionalLight;

class Terrain
{
public:
	Terrain();
	~Terrain();

	bool LoadFromHeightMap(const std::string& heightmapPath, TerrainTechnique* mat, unsigned textures[5], const Vec3& scale);
	bool LoadFromHeightMapWithBillboards(const std::string& heightmapPath, TerrainTechnique* mat, unsigned textures[5], const Vec3& scale, std::vector<Vec3>& billboardPositionsOut, int maxBillboards);

	void Render(Renderer* renderer, Camera* camera, const Vec4& colour);

private:
	TerrainTechnique* m_Material;
	unsigned m_TextureIds[5];
	Vec3 m_Scale;
	GLuint m_VAO;
	GLuint m_VertexVBO;
	GLuint m_IndexVBO;
	int m_Rows;
	int m_Cols;
};

#endif