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

Terrain::Terrain() :
	m_Scale(1.0,1.0,1.0),
	m_VAO(0),
	m_Rows(0),
	m_Cols(0)
{
}

Terrain::~Terrain()
{
	OpenGLLayer::clean_GL_vao(&this->m_VAO, 1);
	OpenGLLayer::clean_GL_buffer(&m_VertexVBO, 1);
	OpenGLLayer::clean_GL_buffer(&m_IndexVBO, 1);
}

bool Terrain::LoadFromHeightMap(const std::string& heightmapPath, ShaderProgram* mat, unsigned textures[5], const Vec3& scale)
{
	if (!mat)
	{
		WRITE_LOG("Material not set for terrain", "error");
		return false;
	}

	for (int i = 0; i < 5; ++i)
	{
		m_TextureIds[i] = textures[i];
	}

	m_Material = mat;
	m_Scale = scale;

	Image heightmap;
	if (!heightmap.LoadImg(heightmapPath.c_str()))
	{
		WRITE_LOG("Height map load failed: " + heightmapPath, "error");
		return false;
	}

	m_Rows = static_cast<int>(heightmap.Height());
	m_Cols = static_cast<int>(heightmap.Width());

	std::vector<std::vector<Vec3>> vVertexData(m_Rows, std::vector<Vec3>(m_Cols));
	std::vector<std::vector<Vec2>> vCoordsData(m_Rows, std::vector<Vec2>(m_Cols));
	std::vector<unsigned> indices;

	float fTextureU = float(m_Cols)*0.1f;
	float fTextureV = float(m_Rows)*0.1f;

	for(int z = 0; z <  m_Rows; ++z)
	{
		for(int x = 0; x < m_Cols; ++x)
		{
			float fScaleC = float(x) / float(m_Cols - 1);
			float fScaleR = float(z) / float(m_Rows - 1);
			//float fVertexHeight = float(*(bDataPointer + row_step*z + x*ptr_inc)) / 255.0f;
			float fVertexHeight = static_cast<float>(*heightmap.GetPixel(x, z)) / 255.0f;

			vVertexData[z][x] = glm::vec3(-0.5f + fScaleC, fVertexHeight, -0.5f + fScaleR);
			vCoordsData[z][x] = glm::vec2(fTextureU*fScaleC, fTextureV*fScaleR);
		}
	}

	// Normals are here - the heightmap contains ( (iRows-1)*(iCols-1) quads, each one containing 2 triangles, therefore array of we have 3D array)
	std::vector<std::vector<Vec3>> vNormals[2];
	for (int i = 0; i < 2; ++i)
		vNormals[i] = std::vector<std::vector<Vec3>>(m_Rows - 1, std::vector<Vec3>(m_Cols - 1));

	for(int i = 0; i < m_Rows - 1; ++i)
	{
		for(int j = 0; j < m_Cols - 1; ++j)
		{
			Vec3 vTriangle0[] =
			{
				vVertexData[i][j],
				vVertexData[i + 1][j],
				vVertexData[i + 1][j + 1]
			};

			Vec3 vTriangle1[] =
			{
				vVertexData[i + 1][j + 1],
				vVertexData[i][j + 1],
				vVertexData[i][j]
			};

			Vec3 vTriangleNorm0 = glm::cross(vTriangle0[0] - vTriangle0[1], vTriangle0[1] - vTriangle0[2]);
			Vec3 vTriangleNorm1 = glm::cross(vTriangle1[0] - vTriangle1[1], vTriangle1[1] - vTriangle1[2]);

			vNormals[0][i][j] = glm::normalize(vTriangleNorm0);
			vNormals[1][i][j] = glm::normalize(vTriangleNorm1);
		}
	}

	// Final normals
	std::vector<std::vector<Vec3>> vFinalNormals = std::vector<std::vector<Vec3>>(m_Rows, std::vector<Vec3>(m_Cols));

	for (int i = 0; i < m_Rows; ++i)
	{
		for (int j = 0; j < m_Cols; ++j)
		{
			// Now we wanna calculate final normal for [i][j] vertex. We will have a look at all triangles this vertex is part of, and then we will make average vector
			// of all adjacent triangles' normals
			Vec3 vFinalNormal = Vec3(0.0f);

			// Look for upper-left triangles
			if (j != 0 && i != 0)
			{
				for (int k = 0; k < 2; ++k)
				{
					vFinalNormal += vNormals[k][i - 1][j - 1];
				}
			}

			// Look for upper-right triangles
			if (i != 0 && j != m_Cols - 1)
			{
				vFinalNormal += vNormals[0][i - 1][j];
			}

			// Look for bottom-right triangles
			if (i != m_Rows - 1 && j != m_Cols - 1)
			{
				for (int k = 0; k < 2; ++k)
				{
					vFinalNormal += vNormals[k][i][j];
				}
			}

			// Look for bottom-left triangles
			if (i != m_Rows - 1 && j != 0)
			{
				vFinalNormal += vNormals[1][i][j - 1];
			}

			vFinalNormal = glm::normalize(vFinalNormal);
			vFinalNormals[i][j] = vFinalNormal; // Store final normal of j-th vertex in i-th row
		}
	}

	// Indices
	int iPrimitiveRestartIndex = m_Rows*m_Cols;
	for(int i = 0; i < m_Rows - 1; ++i)
	{
		for(int j = 0; j < m_Cols; ++j)
		{
			for(int k = 0; k < 2; ++k)
			{
				int iRow = i + (1 - k);
				int iIndex = iRow*m_Cols + j;
				//vboHeightmapIndices.AddData(&iIndex, sizeof(int));
				indices.push_back(iIndex);
			}
		}

		// Restart triangle strips
		indices.push_back(iPrimitiveRestartIndex);
		//vboHeightmapIndices.AddData(&iPrimitiveRestartIndex, sizeof(int));
	}

	std::vector<Vertex> vertices;
	for(int i = 0; i < m_Rows; ++i)
	{
		for (int j = 0; j < m_Cols; ++j)
		{
			Vertex v;
			v.position = vVertexData[i][j];
			v.normal = vFinalNormals[i][j];
			v.texcoord = vCoordsData[i][j];
			vertices.push_back(v);
		}
	}

	// Create GL Buffers
	// Create the VAO
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Create the buffers for the vertices atttributes
	glGenBuffers(1, &m_VertexVBO);
	glGenBuffers(1, &m_IndexVBO);

	// Generate and populate the buffers with vertex attributes and the indices
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)24);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// End
	glBindVertexArray(0);

	return true;
}

bool Terrain::LoadFromHeightMapWithBillboards(const std::string& heightmapPath, ShaderProgram* mat, unsigned textures[5], const Vec3& scale, std::vector<Vec3>& billboardPositionsOut, int maxBillboards)
{
	// TODO :: Refactor
	if (!mat)
	{
		WRITE_LOG("Material not set for terrain", "error");
		return false;
	}

	for (int i = 0; i < 5; ++i)
	{
		m_TextureIds[i] = textures[i];
	}

	m_Material = mat;
	m_Scale = scale;

	Image heightmap;
	if (!heightmap.LoadImg(heightmapPath.c_str()))
	{
		WRITE_LOG("Height map load failed: " + heightmapPath, "error");
		return false;
	}

	m_Rows = static_cast<int>(heightmap.Height());
	m_Cols = static_cast<int>(heightmap.Width());

	std::vector<std::vector<Vec3>> vVertexData(m_Rows, std::vector<Vec3>(m_Cols));
	std::vector<std::vector<Vec2>> vCoordsData(m_Rows, std::vector<Vec2>(m_Cols));
	std::vector<unsigned> indices;

	float fTextureU = float(m_Cols)*0.1f;
	float fTextureV = float(m_Rows)*0.1f;

	int numbills = 0;

	for (int z = 0; z < m_Rows; ++z)
	{
		for (int x = 0; x < m_Cols; ++x)
		{
			float fScaleC = float(x) / float(m_Cols - 1);
			float fScaleR = float(z) / float(m_Rows - 1);
			//float fVertexHeight = float(*(bDataPointer + row_step*z + x*ptr_inc)) / 255.0f;
			float fVertexHeight = static_cast<float>(*heightmap.GetPixel(x, z)) / 255.0f;

			vVertexData[z][x] = glm::vec3(-0.5f + fScaleC, fVertexHeight, -0.5f + fScaleR);
			vCoordsData[z][x] = glm::vec2(fTextureU*fScaleC, fTextureV*fScaleR);
		}
	}

	// Normals are here - the heightmap contains ( (iRows-1)*(iCols-1) quads, each one containing 2 triangles, therefore array of we have 3D array)
	std::vector<std::vector<Vec3>> vNormals[2];
	for (int i = 0; i < 2; ++i)
		vNormals[i] = std::vector<std::vector<Vec3>>(m_Rows - 1, std::vector<Vec3>(m_Cols - 1));

	for (int i = 0; i < m_Rows - 1; ++i)
	{
		for (int j = 0; j < m_Cols - 1; ++j)
		{
			Vec3 vTriangle0[] =
			{
				vVertexData[i][j],
				vVertexData[i + 1][j],
				vVertexData[i + 1][j + 1]
			};

			Vec3 vTriangle1[] =
			{
				vVertexData[i + 1][j + 1],
				vVertexData[i][j + 1],
				vVertexData[i][j]
			};

			Vec3 vTriangleNorm0 = glm::cross(vTriangle0[0] - vTriangle0[1], vTriangle0[1] - vTriangle0[2]);
			Vec3 vTriangleNorm1 = glm::cross(vTriangle1[0] - vTriangle1[1], vTriangle1[1] - vTriangle1[2]);

			vNormals[0][i][j] = glm::normalize(vTriangleNorm0);
			vNormals[1][i][j] = glm::normalize(vTriangleNorm1);
		}
	}

	// Final normals
	std::vector<std::vector<Vec3>> vFinalNormals = std::vector<std::vector<Vec3>>(m_Rows, std::vector<Vec3>(m_Cols));

	for (int i = 0; i < m_Rows; ++i)
	{
		for (int j = 0; j < m_Cols; ++j)
		{
			// Now we wanna calculate final normal for [i][j] vertex. We will have a look at all triangles this vertex is part of, and then we will make average vector
			// of all adjacent triangles' normals
			Vec3 vFinalNormal = Vec3(0.0f);

			// Look for upper-left triangles
			if (j != 0 && i != 0)
			{
				for (int k = 0; k < 2; ++k)
				{
					vFinalNormal += vNormals[k][i - 1][j - 1];
				}
			}

			// Look for upper-right triangles
			if (i != 0 && j != m_Cols - 1)
			{
				vFinalNormal += vNormals[0][i - 1][j];
			}

			// Look for bottom-right triangles
			if (i != m_Rows - 1 && j != m_Cols - 1)
			{
				for (int k = 0; k < 2; ++k)
				{
					vFinalNormal += vNormals[k][i][j];
				}
			}

			// Look for bottom-left triangles
			if (i != m_Rows - 1 && j != 0)
			{
				vFinalNormal += vNormals[1][i][j - 1];
			}

			vFinalNormal = glm::normalize(vFinalNormal);
			vFinalNormals[i][j] = vFinalNormal; // Store final normal of j-th vertex in i-th row
		}
	}

	// Indices
	int iPrimitiveRestartIndex = m_Rows*m_Cols;
	for (int i = 0; i < m_Rows - 1; ++i)
	{
		for (int j = 0; j < m_Cols; ++j)
		{
			for (int k = 0; k < 2; ++k)
			{
				int iRow = i + (1 - k);
				int iIndex = iRow*m_Cols + j;
				//vboHeightmapIndices.AddData(&iIndex, sizeof(int));
				indices.push_back(iIndex);
			}
		}

		// Restart triangle strips
		indices.push_back(iPrimitiveRestartIndex);
		//vboHeightmapIndices.AddData(&iPrimitiveRestartIndex, sizeof(int));
	}

	std::vector<Vertex> vertices;
	for (int i = 0; i < m_Rows; ++i)
	{
		for (int j = 0; j < m_Cols; ++j)
		{
			Vertex v;
			v.position = vVertexData[i][j];
			v.normal = vFinalNormals[i][j];
			v.texcoord = vCoordsData[i][j];
			vertices.push_back(v);
		}
	}

	std::vector<int32> usedList;
	// Create some random billboards (like trees, plants etc)
	for (int i = 0; i < maxBillboards; ++i)
	{
		int32 randomPosition = 0;

		if (usedList.empty())
		{
			randomPosition = random::RandomRange(0, (int32)vertices.size());
			usedList.push_back(randomPosition);
		}
		else
		{
			bool gotNewPos = false;
			while (!gotNewPos)
			{
				randomPosition = random::RandomRange(0, (int32)vertices.size());

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
		billboardPositionsOut.push_back(vertices[usedList[i]].position * m_Scale);
	}

	// Create GL Buffers
	// Create the VAO
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Create the buffers for the vertices atttributes
	glGenBuffers(1, &m_VertexVBO);
	glGenBuffers(1, &m_IndexVBO);

	// Generate and populate the buffers with vertex attributes and the indices
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)24);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// End
	glBindVertexArray(0);

	return true;
}

void Terrain::Render(Renderer* renderer, BaseCamera* cam, const Vec3& colour)
{
	// Use Program
	m_Material->Use();

	for(int i = 0; i <  5; ++i)
	{
		renderer->GetTexture(m_TextureIds[i])->Bind();
	}

	float u = 0.1f * (float)m_Cols;
	float v = 0.1f * (float)m_Rows;

	// Set uniforms per terrain
	m_Material->SetUniformValue<Mat4>("u_WVPXform", &(cam->Projection() * cam->View() * Mat4(1.0f)));
	m_Material->SetUniformValue<Mat4>("u_WorldXform", &(Mat4(1.0f)));
	m_Material->SetUniformValue<Mat4>("u_HeightMapScaleXform", &(glm::scale(Mat4(1.0f), m_Scale)));
	m_Material->SetUniformValue<float>("u_RenderHeight", &m_Scale.y);
	m_Material->SetUniformValue<float>("u_MaxTexU", &u);
	m_Material->SetUniformValue<float>("u_MaxTexV", &v);
	m_Material->SetUniformValue<Vec3>("u_Colour", &colour);

	glBindVertexArray(m_VAO);
	glEnable(GL_PRIMITIVE_RESTART);	// Do we need to disable this?
	glPrimitiveRestartIndex(m_Rows*m_Cols);

	int numIndices = (m_Rows - 1)*m_Cols * 2 + m_Rows - 1;

	glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_INT, 0);
	glDisable(GL_PRIMITIVE_RESTART);
}
