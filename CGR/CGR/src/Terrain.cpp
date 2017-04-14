#include "Terrain.h"

#include <vector>
#include "Image.h"
#include "LogFile.h"
#include "Vertex.h"
#include "Technique.h"
#include "Camera.h"
#include "Lights.h"
#include "Renderer.h"
#include "Texture.h"
#include "OpenGlLayer.h"
#include "utils.h"
#include "Shader.h"
#include "ShaderProgram.h"


float Noise(int x, int y)
{
	// Seed random noise
	int n = x + y * 47;
	n = (n >> 13) ^ n;
	int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
	return 1.0f - ((float)nn / 1073741824.0f);
}

float CosineLerp(float a, float b, float x)
{
	float ft = x * 3.1415927f;
	float f = (1.0f - cos(ft))* 0.5f;
	return a*(1.0f - f) + b*f;
}

float KenPerlin(float xPos, float zPos)
{
	float s, t, u, v;
	s = Noise((int)xPos, (int)zPos);
	t = Noise((int)xPos + 1, (int)zPos);
	u = Noise((int)xPos, (int)zPos + 1);
	v = Noise((int)xPos + 1, (int)zPos + 1);
	float c1 = CosineLerp(s, t, xPos);
	float c2 = CosineLerp(u, v, xPos);

	return CosineLerp(c1, c2, zPos);//Here we use y-yPos, to get the 2nd dimension.
}

glm::vec3 Brownian(const glm::vec3& p, float grid_height, int octaves, float lacunarity, float gain)
{
	/*
	To control and improve the Perlin noise function.
	fruqency: is how many points fit into a provided space
	amplitude: is how tall
	lacunarity: is the rate in which the freq grows
	octaves: is the number of layers and the amount of detail
	*/

	float total = 0.0f;
	float frequency = 1.0f / grid_height;
	float amplitude = gain;

	for (int i = 0; i < octaves; ++i)
	{
		total += KenPerlin((float)p.x * frequency, (float)p.z * frequency) * amplitude;
		frequency *= lacunarity;
		amplitude *= gain;
	}

	//now that we have the value, put it in
	return glm::vec3(p.x, total, p.z);
}

glm::vec3 CalculateBezier(const std::vector<glm::vec3>& cps, float t)
{
	/*
	Bilinear Bzier to be used with 4 control points, we create a temp curve in the BezSurface function
	from the 4 control points that are passed in, the control points are generated from the global UV co-ordinates of the
	terrain mesh, the 4 temp values give us a curve using the u tex coord, we then pass that curve to this function with the value
	along the V tex coord and get the displaced pixel on the curve
	*/
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

glm::vec3 BezierSurface_16(float u, float v, const std::vector<glm::vec3>& Points)
{
	std::vector<glm::vec3> Pu{ 4 };
	// compute 4 control points along u direction
	for (int i = 0; i < 4; ++i)
	{
		std::vector<glm::vec3> curveP{ 4 };
		curveP[0] = Points[i * 4];
		curveP[1] = Points[i * 4 + 1];
		curveP[2] = Points[i * 4 + 2];
		curveP[3] = Points[i * 4 + 3];
		Pu[i] = CalculateBezier(curveP, u);
	}
	// compute final position on the surface using v
	return CalculateBezier(Pu, v);
}

glm::vec3 BezierSurface_4(const std::vector<glm::vec3>& cps, float u, float v)
{
	/*
	Bilinear Bzier to be used with 16 control points, we create a temp curve in the BezSurface function
	from the 16 control points that are passed in, the control points are generated from the global UV co-ordinates of the
	terrain mesh, the 4 temp values give us a curve using the u tex coord, we then pass that curve to this function with the value
	along the V tex coord and get the displaced pixel on the curve
	*/
	std::vector<glm::vec3> curve{ 4 };

	for (size_t j = 0; j < curve.size(); ++j)
	{
		curve[j] = CalculateBezier(cps, u);
	}

	return CalculateBezier(curve, v);
}


SurfaceMesh::SurfaceMesh()
{
}

SurfaceMesh::~SurfaceMesh()
{
	OpenGLLayer::clean_GL_vao(&m_VAO,1);
	OpenGLLayer::clean_GL_buffer(&m_IndexVBO, 1);
	OpenGLLayer::clean_GL_buffer(&m_VertexVBO, 1);
}

void SurfaceMesh::Create(ShaderProgram* mat, size_t materialId, float sizeX, float sizeY, float sizeZ, dword subU, dword subV, int texTileX, int texTileZ, const std::string& heightmap)
{
	m_Material = mat;

	if (!m_Material)
	{
		WRITE_LOG("Material null for surface mesh", "error");
		return;
	}
	else
	{
		m_Material->Use();

		int i = 0;
		m_Material->SetUniformValue<int>("u_LowHeightMap", &i);

		i = 1;
		m_Material->SetUniformValue<int>("u_MediumHeightMap", &i);

		i = 2;
		m_Material->SetUniformValue<int>("u_HighHeightMap", &i);

		i = 3;
		m_Material->SetUniformValue<int>("u_PathMap", &i);

		i = 4;
		m_Material->SetUniformValue<int>("u_PathSampler", &i);
	}

	std::vector<Vertex>		m_Vertices;
	std::vector<unsigned>	m_Indices;

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

				U = ((float)x / x_verts) * (clampUV ? 1.0f : texTileX);
				V = ((float)z / z_verts) * (clampUV ? 1.0f : texTileZ);

				m_Vertices.push_back(
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
				return;
			}

			if (m_Vertices.empty())
			{
				WRITE_LOG("Can't create height map with no vertices for surface mesh", "error");
				return;
			}

			int num_x = subU + 1;
			int num_z = subV + 1;

			for (int z = 0; z < num_z; ++z)
			{
				for (int x = 0; x < num_x; ++x)
				{
					size_t offset = x + z * num_x;
					float height_map_val = static_cast<float>(*(byte*)height_map.GetPixel(x, z));
					//float y_pos = (height_map_val / 1.0f) * sizeY;
					float y_pos = ((1.0f / 255) * height_map_val) * sizeY;


					m_Vertices[offset].position.y = y_pos;
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
		glm::vec3 temp_norm;

		for (dword i = 0; i < m_Indices.size(); i += 3)
		{
			// Get the vertices for each triangle in the element array
			Vec3 p1 = m_Vertices[m_Indices[i]].position;
			Vec3 p2 = m_Vertices[m_Indices[i + 1]].position;
			Vec3 p3 = m_Vertices[m_Indices[i + 2]].position;

			Vec3 u = p2 - p1;
			Vec3 v = p3 - p1;

			temp_norm = glm::cross(u, v);

			// change the new values of normal in the interleaved vertex
			m_Vertices[m_Indices[i]].normal += temp_norm;
			m_Vertices[m_Indices[i + 1]].normal += temp_norm;
			m_Vertices[m_Indices[i + 2]].normal += temp_norm;

		}

		for (size_t v = 0; v < m_Vertices.size(); ++v)
		{
			m_Vertices[v].normal = glm::normalize(m_Vertices[v].normal);
		}
	}

	// Set Buffers
	{
		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		// Create the buffers for the vertices atttributes
		glGenBuffers(1, &m_VertexVBO);
		glGenBuffers(1, &m_IndexVBO);

		// Generate and populate the buffers with vertex attributes and the indices
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_Vertices.size(), m_Vertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)24);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * m_Indices.size(), m_Indices.data(), GL_STATIC_DRAW);

		// End
		glBindVertexArray(0);
	}

	m_NumIndices = m_Indices.size();
	m_MaxHeight = sizeY;
	m_TexU = texTileX;
	m_MaterialId = materialId;
	m_TexV = texTileZ;
}

void SurfaceMesh::CreateBez(
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
	)
{
	m_Material = mat;
	if (!m_Material)
	{
		WRITE_LOG("Material null for surface mesh", "error");
		return;
	}

	Image i;
	if (!i.LoadImg(heightmap.c_str()))
		return;

	std::vector<Vertex> height_map_verts;
	uint32 height_subs = i.Width() - 1;
	float height_x = i.Width();
	float height_z = i.Height();
	float height_y = heightmapSizeY;
	float height_subu = height_x - 1;
	float height_subv = height_z - 1;

	// Create HeightMap Low res
	{
		std::vector<unsigned> height_map_indices;

		// Gen Vertices
		{
			bool clampUV = false;
			int x_verts = height_subu + 1;
			int z_verts = height_subv + 1;

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
					}
					);
				}
			}
		}

		// Gen Heightmap
		{

			if (height_map_verts.empty())
			{
				WRITE_LOG("Can't create height map with no vertices for surface mesh", "error");
				return;
			}

			int num_x = height_subu + 1;
			int num_z = height_subv + 1;

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
			glm::vec3 temp_norm;

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
		std::vector<Vertex>		m_Vertices;
		std::vector<unsigned>	m_Indices;

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

					m_Vertices.push_back(
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
				float X = (m_Vertices[offset].texcoord.x * ((float)height_subu));
				float Y = (m_Vertices[offset].texcoord.y * ((float)height_subv));

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

				m_Vertices[offset].position =
					// Lerp
					0.5f + m_Vertices[offset].position +
					BezierSurface_16(U, V, points);

				patches.clear();
			}
		}

		// Apply Brownian
		{
			if (withBrowian)
			{
				for (size_t v = 0; v < m_Vertices.size(); ++v)
				{
					m_Vertices[v].position.y =
						0.5f + m_Vertices[v].position.y +
						Brownian(m_Vertices[v].position, heightmapSizeY, 8, 2.0f, 0.4f).y;
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
			glm::vec3 temp_norm;

			for (dword i = 0; i < m_Indices.size(); i += 3)
			{
				// Get the vertices for each triangle in the element array
				Vec3 p1 = m_Vertices[m_Indices[i]].position;
				Vec3 p2 = m_Vertices[m_Indices[i + 1]].position;
				Vec3 p3 = m_Vertices[m_Indices[i + 2]].position;

				Vec3 u = p2 - p1;
				Vec3 v = p3 - p1;

				temp_norm = glm::cross(u, v);

				// change the new values of normal in the interleaved vertex
				m_Vertices[m_Indices[i]].normal += temp_norm;
				m_Vertices[m_Indices[i + 1]].normal += temp_norm;
				m_Vertices[m_Indices[i + 2]].normal += temp_norm;

			}

			for (size_t v = 0; v < m_Vertices.size(); ++v)
			{
				m_Vertices[v].normal = glm::normalize(m_Vertices[v].normal);
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

					m_Vertices[x + z * x_verts].texcoord = Vec2(
						(float)x / (x_verts)* tileU,
						(float)z / (z_verts)* tileV
						);
				}
			}
		}

		// Set Buffers
		{
			glGenVertexArrays(1, &m_VAO);
			glBindVertexArray(m_VAO);

			// Create the buffers for the vertices atttributes
			glGenBuffers(1, &m_VertexVBO);
			glGenBuffers(1, &m_IndexVBO);

			// Generate and populate the buffers with vertex attributes and the indices
			glBindBuffer(GL_ARRAY_BUFFER, m_VertexVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_Vertices.size(), m_Vertices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)24);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexVBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * m_Indices.size(), m_Indices.data(), GL_STATIC_DRAW);

			// End
			glBindVertexArray(0);
		}

		m_NumIndices = m_Indices.size();
		m_MaxHeight = heightmapSizeY;
		m_TexU = tileU;
		m_TexV = tileV;
		m_MaterialId = materialId;
	}
}
