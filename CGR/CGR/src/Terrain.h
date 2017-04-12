#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include <string>
#include <vector>
#include "types.h"
#include "gl_headers.h"

// Temp
#include "Image.h"
#include "Vertex.h"

class ShaderProgram;
class Renderer;
class BaseCamera;
struct DirectionalLight;

enum TerrainSamplers
{
	GroundTerrain		= GL_TEXTURE0,		//<-- Use for ground terrain
	MediumTerrain		= GL_TEXTURE1,		//<-- Use for terrain that is medium
	HighTerrain			= GL_TEXTURE2,		//<-- Use for terrain that is high
	PathTexture			= GL_TEXTURE3,		//<-- Use for the actual texture of the path
	PathSampler			= GL_TEXTURE4,		//<-- Use a height map like image where black is the path
	TerrainSamplerCount = 5
};

class SurfaceMesh
{
public:
	SurfaceMesh();
	~SurfaceMesh();

	void Create(ShaderProgram* mat, 
		size_t materialId,
		float sizeX, 
		float size_y, 
		float size_z, 
		dword subU, 
		dword subV, 
		int tile_u, 
		int tile_v, 
		const std::string& heightmap = "");

	void CreateBez(
		ShaderProgram* mat,
		size_t materialId,
		const std::string& heightmap,
		float heightmapSizeY,
		float sizeX,
		float sizeZ,
		dword subU,
		dword subV,
		float tileU,
		float tileV,
		bool withBrowian
		);

private:
	friend class			Renderer;
	size_t					m_MaterialId;
	ShaderProgram*			m_Material;
	GLuint					m_VAO;
	GLuint					m_VertexVBO;
	GLuint					m_IndexVBO;
	unsigned				m_NumIndices;
	float					m_MaxHeight;
	float					m_TexU;
	float					m_TexV;
};

#endif