#include "Terrain.h"

#include <vector>
#include "Image.h"
#include "LogFile.h"
#include "Vertex.h"
#include "Camera.h"
#include "Lights.h"
#include "Renderer.h"
#include "Texture.h"
#include "OpenGlLayer.h"
#include "utils.h"
#include "Shader.h"
#include "ShaderProgram.h"

#include "FpsCamera.h"

namespace bez
{
	float noise(int32 x, int32 y)
	{
		// Seed random noise
		int32 n = x + y * 47;
		n = (n >> 13) ^ n;
		int32 nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
		return 1.0f - ((float)nn / 1073741824.0f);
	}

	float cosineLerp(float a, float b, float x)
	{
		float ft = x * 3.1415927f;
		float f = (1.0f - cos(ft))* 0.5f;
		return a*(1.0f - f) + b*f;
	}

	float kenPerlin(float xPos, float zPos)
	{
		float s, t, u, v;
		s = noise((int)xPos, (int)zPos);
		t = noise((int)xPos + 1, (int)zPos);
		u = noise((int)xPos, (int)zPos + 1);
		v = noise((int)xPos + 1, (int)zPos + 1);
		float c1 = cosineLerp(s, t, xPos);
		float c2 = cosineLerp(u, v, xPos);

		return cosineLerp(c1, c2, zPos); // Here we use y-yPos, to get the 2nd dimension.
	}

	Vec3 brownian(const Vec3& p, float grid_height, int32 octaves, float lacunarity, float gain)
	{
		float total = 0.0f;
		float frequency = 1.0f / grid_height;
		float amplitude = gain;

		for (int i = 0; i < octaves; ++i)
		{
			total += kenPerlin((float)p.x * frequency, (float)p.z * frequency) * amplitude;
			frequency *= lacunarity;
			amplitude *= gain;
		}

		// Now that we have the value, put it in
		return Vec3(p.x, total, p.z);
	}

	Vec3 calculateBezier(const std::vector<Vec3>& cps, float t)
	{
		float temps[4];

		temps[0] = (1 - t) * (1 - t) * (1 - t);
		temps[1] = 3 * t * (1 - t) * (1 - t);
		temps[2] = 3 * (1 - t) * t * t;
		temps[3] = t * t * t;

		return (
			cps[0] * temps[0] +
			cps[1] * temps[1] +
			cps[2] * temps[2] +
			cps[3] * temps[3]
			);
	}

	Vec3 bezierSurface_16(float u, float v, const std::vector<Vec3>& Points)
	{
		std::vector<Vec3> Pu{ 4 };

		// Compute 4 control points along u direction
		for (int i = 0; i < 4; ++i)
		{
			std::vector<Vec3> curveP{ 4 };

			curveP[0] = Points[i * 4];
			curveP[1] = Points[i * 4 + 1];
			curveP[2] = Points[i * 4 + 2];
			curveP[3] = Points[i * 4 + 3];
			Pu[i] = calculateBezier(curveP, u);
		}

		// Compute final position on the surface using v
		return calculateBezier(Pu, v);
	}

	Vec3 bezierSurface_4(const std::vector<Vec3>& cps, float u, float v)
	{
		std::vector<Vec3> curve{ 4 };

		for (size_t j = 0; j < curve.size(); ++j)
		{
			curve[j] = calculateBezier(cps, u);
		}

		return calculateBezier(curve, v);
	}
}

float TerrainConstructor::GetHeightFromPosition(const Vec3& p)
{
	/*
		// Need Inverse of the equation used to gen terrain
		 -- float x_pos = (sizeX / subU) * x;

		E.g. 
		size x = 100
		subU =	 50

		0 : (100 / 50) * 0 = 0;
		1 : (100 / 50) * 1 = 2;
		2 : (100 / 50) * 2 = 4
		3 : (100 / 50) * 3 = 6
		.. etc

		Given: p.x = 4;
		p.x / (sizeX * subU);
		4   / (100   * 50)
		p.x = 6;	<-- X Index
	*/

	// Note, Negate z position
	int x = static_cast<int>(p.x / m_SizeX * m_subU);
	int z = -static_cast<int>(p.z / m_SizeZ * m_subV);

	// Clamp - note the actual width is (subU + 1)
	x = Maths::Clamp(x, 0, (int)m_subU);
	z = Maths::Clamp(z, 0, (int)m_subV);
	
	// Use actual height (suv V + 1)
	int offset = x + z * (m_subV + 1);

	return  m_Vertices[offset].position.y;
}

bool TerrainConstructor::CreateTerrain(
	std::vector<Vertex>& verts_out,
	std::vector<uint32>& indices_out,
	ShaderProgram* shader,
	float sizeX,
	float size_y,
	float sizeZ,
	uint32 subU,
	uint32 subV,
	float tile_u,
	float tile_v,
	const std::string& heightmap)
{
	m_Height = size_y;
	m_TexU = tile_u;
	m_TexV = tile_v;
	m_Shader = shader;
	m_SizeX = sizeX;
	m_SizeZ = sizeZ;
	m_subU = subU;
	m_subV = subV;

	if (!shader)
	{
		WRITE_LOG("Material null for surface mesh", "error");
		return false;
	}
	else
	{
		OnReloadShaders();
	}

	// Gen Vertices
	{
		bool clampUV = false;
		int x_verts = subU + 1;
		int z_verts = subV + 1;

		float U, V;

		for (int z = 0; z < z_verts; ++z)
		{
			for (int x = 0; x < x_verts; ++x)
			{
				// Evenly spaces displacement values in terms of the size of the terrain and the number of sub divisions
				float x_pos = (sizeX / subU) * x;
				float z_pos = (sizeZ / subV) * z;

				U = ((float)x / x_verts) * (clampUV ? 1.0f : tile_u);
				V = ((float)z / z_verts) * (clampUV ? 1.0f : tile_v);

				verts_out.push_back(
					Vertex
				{
					Vec3(static_cast<float>(x_pos), 0.0f, -static_cast<float>(z_pos)),
					Vec3(0.0f),
					Vec2(U, V)
				}
				);
			}
		}
	}

	// Gen Heightmap
	{
		if (!heightmap.empty())
		{
			Image height_map;

			if (!height_map.LoadImg(heightmap.c_str()))
			{
				WRITE_LOG("Can't create height map for surface mesh as loading failed", "error");
				return false;
			}

			if (verts_out.empty())
			{
				WRITE_LOG("Can't create height map with no vertices for surface mesh", "error");
				return false;
			}

			int num_x = subU + 1;
			int num_z = subV + 1;

			for (int z = 0; z < num_z; ++z)
			{
				for (int x = 0; x < num_x; ++x)
				{
					size_t offset = x + z * num_x;
					float height_map_val = static_cast<float>(*(byte*)height_map.GetPixel(x, z));
					float y_pos = ((1.0f / 255) * height_map_val) * size_y;

					verts_out[offset].position.y = y_pos;
				}
			}
		}
	}

	// Gen Indices
	{
		for (dword j = 0; j < subV; ++j)
		{
			dword K = j * (subV + 1);

			for (dword i = 0; i < subU; ++i)
			{
				if (j % 2 == 0)
				{
					if (i % 2 == 0)
					{
						m_Indices.push_back(K + i + subU + 1);
						m_Indices.push_back(K + i);
						m_Indices.push_back(K + i + 1);
						m_Indices.push_back(K + i + subU + 1);
						m_Indices.push_back(K + i + 1);
						m_Indices.push_back(K + i + subU + 2);
					}
					else
					{
						m_Indices.push_back(K + i);
						m_Indices.push_back(K + i + 1);
						m_Indices.push_back(K + i + subU + 2);
						m_Indices.push_back(K + i + subU + 1);
						m_Indices.push_back(K + i);
						m_Indices.push_back(K + i + subU + 2);
					}
				}
				else
				{
					if (i % 2 != 0)
					{
						m_Indices.push_back(K + i + subU + 1);
						m_Indices.push_back(K + i);
						m_Indices.push_back(K + i + 1);
						m_Indices.push_back(K + i + subU + 1);
						m_Indices.push_back(K + i + 1);
						m_Indices.push_back(K + i + subU + 2);
					}
					else
					{
						m_Indices.push_back(K + i);
						m_Indices.push_back(K + i + 1);
						m_Indices.push_back(K + i + subU + 2);
						m_Indices.push_back(K + i + subU + 1);
						m_Indices.push_back(K + i);
						m_Indices.push_back(K + i + subU + 2);
					}
				}
			}
		}
	}

	// Gen Normals
	{
		Vec3 temp_norm;

		for (dword i = 0; i < m_Indices.size(); i += 3)
		{
			// Get the vertices for each triangle in the element array
			Vec3 p1 = verts_out[m_Indices[i]].position;
			Vec3 p2 = verts_out[m_Indices[i + 1]].position;
			Vec3 p3 = verts_out[m_Indices[i + 2]].position;

			Vec3 u = p2 - p1;
			Vec3 v = p3 - p1;

			temp_norm = glm::cross(u, v);

			// change the new values of normal in the interleaved vertex
			verts_out[m_Indices[i]].normal += temp_norm;
			verts_out[m_Indices[i + 1]].normal += temp_norm;
			verts_out[m_Indices[i + 2]].normal += temp_norm;

		}

		for (size_t v = 0; v < verts_out.size(); ++v)
		{
			verts_out[v].normal = glm::normalize(verts_out[v].normal);
		}
	}

	// Copy them locally 
	m_Vertices = verts_out;
	indices_out = m_Indices;

	return true;
}

bool TerrainConstructor::CreateBez(
	std::vector<Vertex>& verts_out,
	std::vector<uint32>& indices_out,
	ShaderProgram* shader,
	const std::string& heightmap,
	float heightmapSizeY,
	float sizeX,
	float sizeZ,
	uint32 subU,
	uint32 subV,
	float tileU,
	float tileV,
	bool withBrowian
)
{
	m_Height = heightmapSizeY;
	m_TexU = tileU;
	m_TexV = tileV;
	m_Shader = shader;
	m_SizeX = sizeX;
	m_SizeZ = sizeZ;
	m_subU = subU;
	m_subV = subV;

	if (!shader)
	{
		WRITE_LOG("Material null for surface mesh", "error");
		return false;
	}
	else
	{
		OnReloadShaders();
	}

	Image i;
	if (!i.LoadImg(heightmap.c_str()))
	{
		WRITE_LOG("Can't create height map for surface mesh as loading failed", "error");
		return false;
	}

	std::vector<Vertex> height_map_verts;
	uint32 height_subs = i.Width() - 1;
	float height_x = static_cast<float>(i.Width());
	float height_z = static_cast<float>(i.Height());
	float height_y = heightmapSizeY;
	dword height_subu = static_cast<dword>(height_x - 1);
	dword height_subv = static_cast<dword>(height_z - 1);

	// Create HeightMap Low res
	{
		std::vector<dword> height_map_indices;

		// Gen Vertices
		{
			bool clampUV = false;
			int x_verts = static_cast<int>(height_subu + 1);
			int z_verts = static_cast<int>(height_subv + 1);

			float U, V;

			for (int z = 0; z < z_verts; ++z)
			{
				for (int x = 0; x < x_verts; ++x)
				{
					// Evenly spaces displacement values in terms of the size of the terrain and the number of sub divisions
					float x_pos = (height_x / height_subu) * x;
					float z_pos = (height_z / height_subv) * z;

					U = ((float)x / x_verts) * (clampUV ? 1.0f : tileU);
					V = ((float)z / z_verts) * (clampUV ? 1.0f : tileV);

					height_map_verts.push_back(
						Vertex
					{
						Vec3(static_cast<float>(x_pos), 0.0f, -static_cast<float>(z_pos)),
						Vec3(0.0f),
						Vec2(U, V)
					});
				}
			}
		}

		// Gen Heightmap
		{
			if (height_map_verts.empty())
			{
				WRITE_LOG("Can't create height map with no vertices for surface mesh", "error");
				return false;
			}

			int num_x = static_cast<int>(height_subu + 1);
			int num_z = static_cast<int>(height_subv + 1);

			for (int z = 0; z < num_z; ++z)
			{
				for (int x = 0; x < num_x; ++x)
				{
					size_t offset = x + z * num_x;
					float height_map_val = static_cast<float>(*(byte*)i.GetPixel(x, z));
					float y_pos = ((1.0f / 255) * height_map_val) * heightmapSizeY;

					height_map_verts[offset].position.y = y_pos;
				}
			}
		}

		// Gen Indices
		{
			for (dword j = 0; j < height_subv; ++j)
			{
				dword K = j * (height_subv + 1);

				for (dword i = 0; i < height_subu; ++i)
				{
					if (j % 2 == 0)
					{
						if (i % 2 == 0)
						{
							height_map_indices.push_back(K + i + height_subu + 1);
							height_map_indices.push_back(K + i);
							height_map_indices.push_back(K + i + 1);
							height_map_indices.push_back(K + i + height_subu + 1);
							height_map_indices.push_back(K + i + 1);
							height_map_indices.push_back(K + i + height_subu + 2);
						}
						else
						{
							height_map_indices.push_back(K + i);
							height_map_indices.push_back(K + i + 1);
							height_map_indices.push_back(K + i + height_subu + 2);
							height_map_indices.push_back(K + i + height_subu + 1);
							height_map_indices.push_back(K + i);
							height_map_indices.push_back(K + i + height_subu + 2);
						}
					}
					else
					{
						if (i % 2 != 0)
						{
							height_map_indices.push_back(K + i + height_subu + 1);
							height_map_indices.push_back(K + i);
							height_map_indices.push_back(K + i + 1);
							height_map_indices.push_back(K + i + height_subu + 1);
							height_map_indices.push_back(K + i + 1);
							height_map_indices.push_back(K + i + height_subu + 2);
						}
						else
						{
							height_map_indices.push_back(K + i);
							height_map_indices.push_back(K + i + 1);
							height_map_indices.push_back(K + i + height_subu + 2);
							height_map_indices.push_back(K + i + height_subu + 1);
							height_map_indices.push_back(K + i);
							height_map_indices.push_back(K + i + height_subu + 2);
						}
					}
				}
			}
		}

		// Gen Normals
		{
			Vec3 temp_norm;

			for (dword i = 0; i < height_map_indices.size(); i += 3)
			{
				// Get the vertices for each triangle in the element array
				Vec3 p1 = height_map_verts[height_map_indices[i]].position;
				Vec3 p2 = height_map_verts[height_map_indices[i + 1]].position;
				Vec3 p3 = height_map_verts[height_map_indices[i + 2]].position;

				Vec3 u = p2 - p1;
				Vec3 v = p3 - p1;

				temp_norm = glm::cross(u, v);

				// change the new values of normal in the interleaved vertex
				height_map_verts[height_map_indices[i]].normal += temp_norm;
				height_map_verts[height_map_indices[i + 1]].normal += temp_norm;
				height_map_verts[height_map_indices[i + 2]].normal += temp_norm;
			}

			for (size_t v = 0; v < height_map_verts.size(); ++v)
			{
				height_map_verts[v].normal = glm::normalize(height_map_verts[v].normal);
			}
		}
	}

	// Now Apply to upscaled
	{
		std::vector< std::vector<Vec3> >patches;
		std::vector<Vec3> points{ 16 };

		// Gen Vertices
		{
			bool clampUV = true;
			int x_verts = subU + 1;
			int z_verts = subV + 1;

			float U, V;

			for (int z = 0; z < z_verts; ++z)
			{
				for (int x = 0; x < x_verts; ++x)
				{
					// Evenly spaces displacement values in terms of the size of the terrain and the number of sub divisions
					float x_pos = (sizeX / subU) * x;
					float z_pos = (sizeZ / subV) * z;

					U = ((float)x / x_verts) * (clampUV ? 1.0f : tileU);
					V = ((float)z / z_verts) * (clampUV ? 1.0f : tileV);

					verts_out.push_back(
						Vertex
					{
						Vec3(static_cast<float>(x_pos), 0.0f, -static_cast<float>(z_pos)),
						Vec3(0.0f),
						Vec2(U, V)
					}
					);
				}
			}
		}

		size_t x_verts = subU + 1;
		size_t z_verts = subV + 1;

		// Displace Bezier
		for (size_t y = 0; y < z_verts; ++y)
		{
			for (size_t x = 0; x < x_verts; ++x)
			{
				// Loop through each vertex in order with offset
				size_t offset = x + y * x_verts;

				// Algorithm I created to get first control point, use global uv and multiply by the amount of subs in cps
				float X = (verts_out[offset].texcoord.x * ((float)height_subu));
				float Y = (verts_out[offset].texcoord.y * ((float)height_subv));

				// Use this for patches that share control points to lerp and smooth them
				size_t x_patch_offset = (size_t)X - (size_t)X % 3;
				size_t z_patch_offset = (size_t)Y - (size_t)Y % 3;
				size_t patch_id = x_patch_offset + z_patch_offset * (size_t)height_x;

				// This is calculated for smooth local UV and passed to bezier function along with contro points for this vertex
				float U = (X - x_patch_offset) / 3;
				float V = (Y - z_patch_offset) / 3;

				// Generate 16 cps ( 4 rows ), using the above algorithm 
				points[0] = height_map_verts[patch_id].position;
				points[1] = height_map_verts[patch_id + 1].position;
				points[2] = height_map_verts[patch_id + 2].position;
				points[3] = height_map_verts[patch_id + 3].position;

				points[4] = height_map_verts[patch_id + (size_t)height_x].position;
				points[5] = height_map_verts[patch_id + (size_t)height_x + 1].position;
				points[6] = height_map_verts[patch_id + (size_t)height_x + 2].position;
				points[7] = height_map_verts[patch_id + (size_t)height_x + 3].position;

				points[8] = height_map_verts[patch_id + ((size_t)height_x * 2)].position;
				points[9] = height_map_verts[patch_id + ((size_t)height_x * 2) + 1].position;
				points[10] = height_map_verts[patch_id + ((size_t)height_x * 2) + 2].position;
				points[11] = height_map_verts[patch_id + ((size_t)height_x * 2) + 3].position;

				points[12] = height_map_verts[patch_id + ((size_t)height_x * 3)].position;
				points[13] = height_map_verts[patch_id + ((size_t)height_x * 3) + 1].position;
				points[14] = height_map_verts[patch_id + ((size_t)height_x * 3) + 2].position;
				points[15] = height_map_verts[patch_id + ((size_t)height_x * 3) + 3].position;

				verts_out[offset].position =
					// Lerp
					//0.5f + 
					verts_out[offset].position +
					bez::bezierSurface_16(U, V, points);

				patches.clear();
			}
		}

		// Apply Brownian
		{
			if (withBrowian)
			{
				for (size_t v = 0; v < verts_out.size(); ++v)
				{
					verts_out[v].position.y =
						0.5f + verts_out[v].position.y +
						bez::brownian(verts_out[v].position, heightmapSizeY, 8, 2.0f, 0.4f).y;
				}
			}
		}

		// Gen Indices
		{
			for (dword j = 0; j < subV; ++j)
			{
				dword K = j * (subV + 1);

				for (dword i = 0; i < subU; ++i)
				{
					if (j % 2 == 0)
					{
						if (i % 2 == 0)
						{
							m_Indices.push_back(K + i + subU + 1);
							m_Indices.push_back(K + i);
							m_Indices.push_back(K + i + 1);
							m_Indices.push_back(K + i + subU + 1);
							m_Indices.push_back(K + i + 1);
							m_Indices.push_back(K + i + subU + 2);
						}
						else
						{
							m_Indices.push_back(K + i);
							m_Indices.push_back(K + i + 1);
							m_Indices.push_back(K + i + subU + 2);
							m_Indices.push_back(K + i + subU + 1);
							m_Indices.push_back(K + i);
							m_Indices.push_back(K + i + subU + 2);
						}
					}
					else
					{
						if (i % 2 != 0)
						{
							m_Indices.push_back(K + i + subU + 1);
							m_Indices.push_back(K + i);
							m_Indices.push_back(K + i + 1);
							m_Indices.push_back(K + i + subU + 1);
							m_Indices.push_back(K + i + 1);
							m_Indices.push_back(K + i + subU + 2);
						}
						else
						{
							m_Indices.push_back(K + i);
							m_Indices.push_back(K + i + 1);
							m_Indices.push_back(K + i + subU + 2);
							m_Indices.push_back(K + i + subU + 1);
							m_Indices.push_back(K + i);
							m_Indices.push_back(K + i + subU + 2);
						}
					}
				}
			}
		}

		// Gen Normals
		{
			Vec3 temp_norm;

			for (dword i = 0; i < m_Indices.size(); i += 3)
			{
				// Get the vertices for each triangle in the element array
				Vec3 p1 = verts_out[m_Indices[i]].position;
				Vec3 p2 = verts_out[m_Indices[i + 1]].position;
				Vec3 p3 = verts_out[m_Indices[i + 2]].position;

				Vec3 u = p2 - p1;
				Vec3 v = p3 - p1;

				temp_norm = glm::cross(u, v);

				// change the new values of normal in the interleaved vertex
				verts_out[m_Indices[i]].normal += temp_norm;
				verts_out[m_Indices[i + 1]].normal += temp_norm;
				verts_out[m_Indices[i + 2]].normal += temp_norm;

			}

			for (size_t v = 0; v < verts_out.size(); ++v)
			{
				verts_out[v].normal = glm::normalize(verts_out[v].normal);
			}
		}

		// Reapply this to sort texcoords
		{
			int x_verts = subU + 1;
			int z_verts = subV + 1;
			for (int z = 0; z < z_verts; ++z)
			{
				for (int x = 0; x < x_verts; ++x)
				{
					// Evenly spaces displacement values in terms of the size of the terrain and the number of sub divisions
					float x_pos = (sizeX / subU) * x;
					float z_pos = (sizeZ / subV) * z;

					verts_out[x + z * x_verts].texcoord =
						Vec2((float)x / (x_verts)* tileU,
						(float)z / (z_verts)* tileV
						);
				}
			}
		}

	}

	// Copy them locally 
	m_Vertices = verts_out;
	indices_out = m_Indices;
	return true;
}

void TerrainConstructor::GenerateRandomPositions(const std::vector<Vertex>& vertsIN, std::vector<Vec3>& positionsOUT, int maxPositions)
{
	int maxBillboards = Maths::Min(maxPositions, (int)vertsIN.size());
	
	std::vector<int32> usedList;

	// Create some random billboards (like trees, plants etc)
	for (int i = 0; i < maxBillboards; ++i)
	{
		int32 randomPosition = 0;

		if (usedList.empty())
		{
			randomPosition = random::RandomRange(0, (int32)vertsIN.size());
			usedList.push_back(randomPosition);
		}
		else
		{
			bool gotNewPos = false;
			while (!gotNewPos)
			{
				randomPosition = random::RandomRange(0, (int32)vertsIN.size());

				bool same = false;
				for (int j = 0; j < usedList.size(); ++j)
				{
					if (randomPosition == usedList[j])
					{
						same = true;
						break;
					}
				}

				if (!same)
				{
					usedList.push_back(randomPosition);
					gotNewPos = true;
				}
			}
		}
	}

	for (int i = 0; i < usedList.size(); ++i)
	{
		positionsOUT.push_back(Vec3(
			vertsIN[usedList[i]].position.x,
			vertsIN[usedList[i]].position.y - 0.25f,
			vertsIN[usedList[i]].position.z)
		);
	}
}

float TerrainConstructor::GetHeight() const
{
	return m_Height;
}

float TerrainConstructor::GetTexU() const
{
	return m_TexU;
}

float TerrainConstructor::GetTexV() const
{
	return m_TexV;
}

float TerrainConstructor::GetSizeX() const
{
	return m_SizeX;
}

float TerrainConstructor::GetSizeZ() const
{
	return m_SizeZ;
}

void  TerrainConstructor::OnReloadShaders()
{
	if (m_Shader)
	{
		m_Shader->Use();

		// Set Material uniforms
		int i = 0;
		m_Shader->SetUniformValue<int>("u_LowHeightMap", &i);

		i = 1;
		m_Shader->SetUniformValue<int>("u_MediumHeightMap", &i);

		i = 2;
		m_Shader->SetUniformValue<int>("u_HighHeightMap", &i);

		i = 3;
		m_Shader->SetUniformValue<int>("u_PathMap", &i);

		i = 4;
		m_Shader->SetUniformValue<int>("u_PathSampler", &i);

		i = 6;
		m_Shader->SetUniformValue<int>("u_shadow_sampler", &i);

		// This means can only have one mesh
		m_Shader->SetUniformValue<float>("u_MaxHeight", &m_Height);
		m_Shader->SetUniformValue<float>("u_MaxTexU", &m_TexU);
		m_Shader->SetUniformValue<float>("u_MaxTexV", &m_TexV);
	}
}


// ---- Collision ----
const Vec3 GRAVITY = Vec3(0, -0.981f, 0);
const float UNITS_PER_METRE = 100.0f;
const float UNIT_SCALE = UNITS_PER_METRE / 100.0f;
const float VERY_CLOSE_DIST = 0.005f * UNIT_SCALE;

Vec3 TerrainConstructor::CollisionSlide(CollisionPacket& cP)
{
	// Transform velocity vector to the ellipsoid space (e_ denotes ellipsoid space)
	cP.e_vel = cP.w_vel / cP.ellipsoidSpace;

	// Transform position vector to the ellipsoid space
	cP.e_pos = cP.w_pos / cP.ellipsoidSpace;

	// Now we check for a collision with our world, this function will
	// call itself 5 times at most, or until the velocity vector is
	// used up (very small (near zero to zero length))
	cP.collision_recursion_depth = 0;
	Vec3 finalPosition = CollideWithWorld(cP);

	// Add gravity pull:
	cP.e_vel = GRAVITY / cP.ellipsoidSpace;	// We defined gravity in world space, so now we have
											// to convert it to ellipsoid space
	cP.e_pos = finalPosition;
	cP.collision_recursion_depth = 0;
	finalPosition = CollideWithWorld(cP);

	// Convert our final position from ellipsoid space to world space
	finalPosition = finalPosition * cP.ellipsoidSpace;

	// Return our final position!
	return finalPosition;
}

Vec3 TerrainConstructor::CollideWithWorld(CollisionPacket& colpak)
{
	// Prevent infinite loop
	if (colpak.collision_recursion_depth > 5)
		return colpak.e_pos;

	// Normalize
	colpak.e_norm_vel = glm::normalize(colpak.e_vel);

	colpak.found_collision = false;
	colpak.nearest_distance = 0.0f;
	
	// Loop polygons
	for (int i = 0; i < m_Indices.size() - 3; i += 3)
		{
			Vec3 p0(m_Vertices[m_Indices[i]].position);
			Vec3 p1(m_Vertices[m_Indices[i + 1]].position);
			Vec3 p2(m_Vertices[m_Indices[i + 2]].position);

			if (glm::distance(colpak.w_pos, (p0 + p2) * 0.5f) > 15.0f)
				continue;

			// Convert triangle into elipsoid space
			p0 = p0 / colpak.ellipsoidSpace;
			p1 = p1 / colpak.ellipsoidSpace;
			p2 = p2 / colpak.ellipsoidSpace;

			// Calc normal for this triangle
			Vec3 tri_norm = glm::normalize(glm::cross(p1 - p0, p2 - p0));

			// Check if sphere is colliding with triange
			SphereCollidingWithTriangle(colpak, p0, p1, p2, tri_norm);
		}

	// If no collision return position + velocity
	if (colpak.found_collision == false)
	{
		return colpak.e_pos + colpak.e_vel;
	}

	// A Collision has occured
	// destinationPoint is where the sphere would travel if there was
	// no collisions, however, at this point, there has a been a collision
	// detected. We will use this vector to find the new "sliding" vector
	// based off the plane created from the sphere and collision point
	Vec3 dest_point = colpak.e_pos + colpak.e_vel;
	Vec3 new_pos = colpak.e_pos;

	if (colpak.nearest_distance >= VERY_CLOSE_DIST)
	{
		// Move the new position down velocity vector to ALMOST touch the collision point
		Vec3 v = glm::normalize(colpak.e_vel);
		v *= (colpak.nearest_distance - VERY_CLOSE_DIST);
		new_pos = colpak.e_pos + v;

		// Adjust polygon intersection point (so sliding
		// plane will be unaffected by the fact that we
		// move slightly less than collision tells us)
		v = glm::normalize(v);
		colpak.intersection_point -= VERY_CLOSE_DIST * v;
	}

	// Sliding point in plane
	Vec3 slide_plane_origin = colpak.intersection_point;
	Vec3 slide_plane_norm = glm::normalize(new_pos - colpak.intersection_point);

	// Use slide plane to compute new dest point
	float x = slide_plane_origin.x;
	float y = slide_plane_origin.y;
	float z = slide_plane_origin.z;

	// Plane normal
	float A = slide_plane_norm.x;
	float B = slide_plane_norm.y;
	float C = slide_plane_norm.z;
	float D = -((A * x) + (B * y) + (C * z));

	float plane_constant = D;

	float signedDistFromDestPointToSlidingP = (glm::dot(dest_point, slide_plane_norm)) + plane_constant;

	Vec3 new_dest_point = dest_point - signedDistFromDestPointToSlidingP * slide_plane_norm;
	Vec3 new_vel = new_dest_point - colpak.intersection_point;

	// After this check, we will recurse. This check makes sure that we have not
	// come to the end of our velocity vector (or very close to it, because if the velocity
	// vector is very small, there is no reason to lose performance by doing an extra recurse
	// when we won't even notice the distance "thrown away" by this check anyway) before
	// we recurse
	if (glm::length(new_vel) <  VERY_CLOSE_DIST)
	{
		return new_pos;
	}

	// We are going to recurse now since a collision was found and the velocity
	// changed directions. we need to check if the new velocity vector will
	// cause the sphere to collide with other geometry.
	colpak.collision_recursion_depth++;
	colpak.e_pos = new_pos;
	colpak.e_vel = new_vel;

	return CollideWithWorld(colpak);
}

bool TerrainConstructor::SphereCollidingWithTriangle(CollisionPacket& cP, const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& tri_norm)
{
	float facing = glm::dot(tri_norm, cP.e_norm_vel);

	if (facing <= 0)
	{
		Vec3 velocity = cP.e_vel;
		Vec3 pos = cP.e_pos;

		float t0 = 0.0f, t1 = 0.0f;

		bool sphere_in_plane = false;

		float x = p0.x;
		float y = p0.y;
		float z = p0.z;

		float A = tri_norm.x;
		float B = tri_norm.y;
		float C = tri_norm.z;
		float D = -((A * x) + (B * y) + (C * z));

		float planeConstant = D;
		float signedDist = glm::dot(pos, tri_norm) + planeConstant;
		float planeNormalDotVel = glm::dot(tri_norm, velocity);

		if (planeNormalDotVel == 0.0f)
		{
			//FABS?
			if (fabs(signedDist) >= 1.0f)
			{
				return false;
			}
			else
			{
				sphere_in_plane = true;
			}
		}
		else
		{
			t0 = (1.0f - signedDist) / planeNormalDotVel;
			t1 = (-1.0f - signedDist) / planeNormalDotVel;
			// We will make sure that t0 is smaller than t1, which means that t0 is when the sphere FIRST
			// touches the planes surface
			if (t0 > t1)
			{
				float temp = t0;
				t0 = t1;
				t1 = temp;
			}

			// If the swept sphere touches the plane outside of the 0 to 1 "timeframe", we know that
			// the sphere is not going to intersect with the plane (and of course triangle) this frame
			if (t0 > 1.0f || t1 < 0.0f)
			{
				return false;
			}

			// If t0 is smaller than 0 then we will make it 0
			// and if t1 is greater than 1 we will make it 1
			if (t0 < 0.0f) t0 = 0.0f;
			if (t1 > 1.0f) t1 = 1.0f;


		}

		Vec3 collisionPoint = Vec3(0.0f);
		bool collidingWithTri = false;
		float t = 1.0f;

		if (!sphere_in_plane)
		{
			Vec3 planeIntersectionPoint = (pos + t0 * velocity - tri_norm);

			if (CheckPointInTriangle(planeIntersectionPoint, p0, p1, p2))
			{
				collidingWithTri = true;
				t = t0;
				collisionPoint = planeIntersectionPoint;
			}
		}

		if (collidingWithTri == false)
		{
			float a, b, c;

			// We can use the squared velocities length below when checking for collisions with the edges of the triangles
			// to, so to keep things clear, we won't set a directly
			float velocityLengthSquared = glm::length(velocity);// velocity.Length();
			velocityLengthSquared *= velocityLengthSquared;

			// We'll start by setting 'a', since all 3 point equations use this 'a'
			a = velocityLengthSquared;

			// This is a temporary variable to hold the distance down the velocity vector that
			// the sphere will touch the vertex.
			float newT = 0.0f;

			// P0 - Collision test with sphere and p0
			b = 2.0f * glm::dot(velocity, pos - p0);
			Vec3 temp = p0 - pos;
			c = glm::length(temp);// temp.Length();
			c = (c * c) - 1.0f;
			if (GetLowestRoot(a, b, c, t, newT))
			{	// Check if the equation can be solved
				// If the equation was solved, we can set a couple things. First we set t (distance
				// down velocity vector the sphere first collides with vertex) to the temporary newT,
				// Then we set collidingWithTri to be true so we know there was for sure a collision
				// with the triangle, then we set the exact point the sphere collides with the triangle,
				// which is the position of the vertex it collides with
				t = newT;
				collidingWithTri = true;
				collisionPoint = p0;
			}

			// P1 - Collision test with sphere and p1
			b = 2.0f * glm::dot(velocity, pos - p1);
			Vec3 P = p1 - pos;
			c = glm::length(P);// P.Length();
			c = (c*c) - 1.0f;
			if (GetLowestRoot(a, b, c, t, newT))
			{
				t = newT;
				collidingWithTri = true;
				collisionPoint = p1;
			}

			// P2 - Collision test with sphere and p2
			b = 2.0f * glm::dot(velocity, pos - p2);
			Vec3 Q = p2 - pos;
			c = glm::length(Q);// Q.Length();
			c = (c*c) - 1.0f;
			if (GetLowestRoot(a, b, c, t, newT))
			{
				t = newT;
				collidingWithTri = true;
				collisionPoint = p2;
			}
			//////////////////////////////////////////////Sphere-Edge Collision Test//////////////////////////////////////////////
			// Even though there might have been a collision with a vertex, we will still check for a collision with an edge of the
			// triangle in case an edge was hit before the vertex. Again we will solve a quadratic equation to find where (and if)
			// the swept sphere's position is 1 unit away from the edge of the triangle. The equation parameters this time are a 
			// bit more complex: (still "Ax^2 + Bx + C = 0")
			// a = edgeLength^2 * -velocityLength^2 + (edge . velocity)^2
			// b = edgeLength^2 * 2(velocity . spherePositionToVertex) - 2((edge . velocity)(edge . spherePositionToVertex))
			// c =  edgeLength^2 * (1 - spherePositionToVertexLength^2) + (edge . spherePositionToVertex)^2
			// . denotes dot product

			// Edge (p0, p1):
			Vec3 edge = p1 - p0;
			Vec3 spherePositionToVertex = p0 - pos;
			float edgeLengthSquared = glm::length(edge);// edge.Length();
			edgeLengthSquared *= edgeLengthSquared;
			float edgeDotVelocity = glm::dot(edge, velocity);
			float edgeDotSpherePositionToVertex = glm::dot(edge, spherePositionToVertex);
			float spherePositionToVertexLengthSquared = glm::length(spherePositionToVertex);// spherePositionToVertex.Length();
			spherePositionToVertexLengthSquared = spherePositionToVertexLengthSquared * spherePositionToVertexLengthSquared;

			// Equation parameters
			a = edgeLengthSquared * -velocityLengthSquared + (edgeDotVelocity * edgeDotVelocity);
			b = edgeLengthSquared * (2.0f * glm::dot(velocity, spherePositionToVertex)) - (2.0f * edgeDotVelocity * edgeDotSpherePositionToVertex);
			c = edgeLengthSquared * (1.0f - spherePositionToVertexLengthSquared) + (edgeDotSpherePositionToVertex * edgeDotSpherePositionToVertex);

			// We start by finding if the swept sphere collides with the edges "infinite line"
			if (GetLowestRoot(a, b, c, t, newT))
			{
				// Now we check to see if the collision happened between the two vertices that make up this edge
				// We can calculate where on the line the collision happens by doing this:
				// f = (edge . velocity)newT - (edge . spherePositionToVertex) / edgeLength^2
				// if f is between 0 and 1, then we know the collision happened between p0 and p1
				// If the collision happened at p0, the f = 0, if the collision happened at p1 then f = 1
				float f = (edgeDotVelocity * newT - edgeDotSpherePositionToVertex) / edgeLengthSquared;
				if (f >= 0.0f && f <= 1.0f)
				{
					// If the collision with the edge happened, we set the results
					t = newT;
					collidingWithTri = true;
					collisionPoint = p0 + f * edge;
				}
			}

			// Edge (p1, p2):
			edge = p2 - p1;
			spherePositionToVertex = p1 - pos;
			edgeLengthSquared = glm::length(edge);// edge.Length();
			edgeLengthSquared = edgeLengthSquared * edgeLengthSquared;
			edgeDotVelocity = glm::dot(edge, cP.e_vel);
			edgeDotSpherePositionToVertex = glm::dot(edge, spherePositionToVertex);
			spherePositionToVertexLengthSquared = glm::length(spherePositionToVertex);// spherePositionToVertex.Length();
			spherePositionToVertexLengthSquared = spherePositionToVertexLengthSquared * spherePositionToVertexLengthSquared;

			a = edgeLengthSquared * -velocityLengthSquared + (edgeDotVelocity * edgeDotVelocity);
			b = edgeLengthSquared * (2.0f * glm::dot(velocity, spherePositionToVertex)) - (2.0f * edgeDotVelocity * edgeDotSpherePositionToVertex);
			c = edgeLengthSquared * (1.0f - spherePositionToVertexLengthSquared) + (edgeDotSpherePositionToVertex * edgeDotSpherePositionToVertex);

			if (GetLowestRoot(a, b, c, t, newT))
			{
				float f = (edgeDotVelocity * newT - edgeDotSpherePositionToVertex) / edgeLengthSquared;
				if (f >= 0.0f && f <= 1.0f)
				{
					t = newT;
					collidingWithTri = true;
					collisionPoint = p1 + f * edge;
				}
			}

			// Edge (p2, p0):
			edge = p0 - p2;
			spherePositionToVertex = p2 - pos;
			edgeLengthSquared = glm::length(edge);// Squared * edgeLengthSquared;
			edgeDotVelocity = glm::dot(edge, velocity);
			edgeDotSpherePositionToVertex = glm::dot(edge, spherePositionToVertex);
			spherePositionToVertexLengthSquared = glm::length(spherePositionToVertex);// spherePositionToVertex.Length();
			spherePositionToVertexLengthSquared = spherePositionToVertexLengthSquared * spherePositionToVertexLengthSquared;

			a = edgeLengthSquared * -velocityLengthSquared + (edgeDotVelocity * edgeDotVelocity);
			b = edgeLengthSquared * (2.0f * glm::dot(velocity, spherePositionToVertex)) - (2.0f * edgeDotVelocity * edgeDotSpherePositionToVertex);
			c = edgeLengthSquared * (1.0f - spherePositionToVertexLengthSquared) + (edgeDotSpherePositionToVertex * edgeDotSpherePositionToVertex);

			if (GetLowestRoot(a, b, c, t, newT))
			{
				float f = (edgeDotVelocity * newT - edgeDotSpherePositionToVertex) / edgeLengthSquared;
				if (f >= 0.0f && f <= 1.0f)
				{
					t = newT;
					collidingWithTri = true;
					collisionPoint = p2 + f * edge;
				}
			}
		}

		// If we have found a collision, we will set the results of the collision here
		if (collidingWithTri == true)
		{
			// We find the distance to the collision using the time variable (t) times the length of the velocity vector
			float distToCollision = t * glm::length(velocity);// velocity.Length();

			// Now we check if this is the first triangle that has been collided with OR it is 
			// the closest triangle yet that was collided with
			if (cP.found_collision == false || distToCollision < cP.nearest_distance)
			{

				// Collision response information (used for "sliding")
				cP.nearest_distance = distToCollision;
				cP.intersection_point = collisionPoint;

				// Make sure this is set to true if we've made it this far
				cP.found_collision = true;
				return true;
			}
		}
	}
	return false;
}

bool TerrainConstructor::CheckPointInTriangle(const Vec3& point, const Vec3& tri_p1, const Vec3& tri_p2, const Vec3& tri_p3)
{
	Vec3 cp1 = glm::cross((tri_p3 - tri_p2), (point - tri_p2));
	Vec3 cp2 = glm::cross((tri_p3 - tri_p2), (tri_p1 - tri_p2));
	if (glm::dot(cp1, cp2) >= 0)
	{
		cp1 = glm::cross((tri_p3 - tri_p1), (point - tri_p1));
		cp2 = glm::cross((tri_p3 - tri_p1), (tri_p2 - tri_p1));
		if (glm::dot(cp1, cp2) >= 0)
		{
			cp1 = glm::cross((tri_p2 - tri_p1), (point - tri_p1));
			cp2 = glm::cross((tri_p2 - tri_p1), (tri_p3 - tri_p1));
			if (glm::dot(cp1, cp2) >= 0)
			{
				return true;
			}
		}
	}
	return false;
}

bool TerrainConstructor::GetLowestRoot(float a, float b, float c, float MAX, float& root)
{
	// Check if a solution exists
	float determinant = b * b - 4.0f * a * c;

	// If determinant is negative it means no solutions.
	if (determinant < 0.0f) return false;

	// calculate the two roots: (if determinant == 0 then
	// x1==x2 but lets disregard that slight optimization)
	float sqrtD = sqrtf(determinant);
	float r1 = (-b - sqrtD) / (2 * a);
	float r2 = (-b + sqrtD) / (2 * a);
	
	// Sort so x1 <= x2
	if (r1 > r2)
	{
		float temp = r2;
		r2 = r1;
		r1 = temp;
	}
	// Get lowest root:
	if (r1 > 0 && r1 < MAX)
	{
		root = r1;
		return true;
	}
	// It is possible that we want x2 - this can happen
	// if x1 < 0
	if (r2 > 0 && r2 < MAX)
	{
		root = r2;
		return true;
	}

	// No (valid) solutions
	return false;
}
	