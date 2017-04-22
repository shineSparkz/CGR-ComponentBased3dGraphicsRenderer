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
	std::vector<uint32>& m_Indices,
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

	return true;
}

bool TerrainConstructor::CreateBez(
	std::vector<Vertex>& verts_out,
	std::vector<uint32>& m_Indices,
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
