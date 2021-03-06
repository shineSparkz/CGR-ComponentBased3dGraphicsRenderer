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
struct CollisionPacket;

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
	float GetSizeX() const;
	float GetSizeZ() const;
	void  OnReloadShaders();

	float GetHeightFromPosition(const Vec3& p);

	// Collision stuff
	Vec3 CollisionSlide(CollisionPacket& cP);
	Vec3 CollideWithWorld(CollisionPacket& colpak);
	bool SphereCollidingWithTriangle(CollisionPacket& cP, const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& tri_norm);
	bool CheckPointInTriangle(const Vec3& point, const Vec3& tri_p1, const Vec3& tri_p2, const Vec3& tri_p3);
	bool GetLowestRoot(float a, float b, float c, float MAX, float& root);


private:
	std::vector<Vertex>		m_Vertices;
	std::vector<unsigned>	m_Indices;
	ShaderProgram*			m_Shader;	//<-- Weak Ptr
	float					m_Height;
	float					m_TexU;
	float					m_TexV;
	uint32					m_subU;
	uint32					m_subV;
	float					m_SizeX;
	float					m_SizeZ;
};

#endif