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

class TerrainConstructor
{
public:
	bool CreateTerrain(
		std::vector<Vertex>& vertsOut,
		std::vector<uint32>& indicesOut,
		ShaderProgram* shader,
		float sizeX,
		float size_y,
		float size_z,
		uint32 subU,
		uint32 subV,
		float tile_u,
		float tile_v,
		const std::string& heightmap = ""
	);


	bool CreateBez(
		std::vector<Vertex>& vertsOut,
		std::vector<uint32>& indicesOut,
		ShaderProgram* mat,
		const std::string& heightmap,
		float heightmapSizeY,
		float sizeX,
		float sizeZ,
		uint32 subU,
		uint32 subV,
		float tileU,
		float tileV,
		bool withBrowian
	);

	void GenerateRandomPositions(const std::vector<Vertex>& vertsIN, std::vector<Vec3>& positionsOUT, int maxPositions);

	float GetHeight() const;
	float GetTexU() const;
	float GetTexV() const;
	void  OnReloadShaders();

private:
	ShaderProgram*	m_Shader;	//<-- Weak Ptr
	float			m_Height;
	float			m_TexU;
	float			m_TexV;
};

#endif