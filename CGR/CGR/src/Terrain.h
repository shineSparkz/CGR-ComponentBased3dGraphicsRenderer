#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include <string>
#include <vector>
#include "types.h"
#include "gl_headers.h"

class ShaderProgram;
class Renderer;
class BaseCamera;
struct DirectionalLight;

class Terrain
{
public:
	Terrain();
	~Terrain();

	bool LoadFromHeightMap(const std::string& heightmapPath, ShaderProgram* mat, unsigned textures[5], const Vec3& scale, const Vec3& colour);
	bool LoadFromHeightMapWithBillboards(const std::string& heightmapPath, ShaderProgram* mat, unsigned textures[5], const Vec3& scale, const Vec3& colour, std::vector<Vec3>& billboardPositionsOut, int maxBillboards);

private:
	void setStaticShaderUniforms(const Vec3& scale, const Vec3& colour);

private:
	friend class Renderer;
	unsigned m_TextureIds[5];
	ShaderProgram* m_Material;
	GLuint m_VAO;
	GLuint m_VertexVBO;
	GLuint m_IndexVBO;
	int m_Rows;
	int m_Cols;
};

#endif