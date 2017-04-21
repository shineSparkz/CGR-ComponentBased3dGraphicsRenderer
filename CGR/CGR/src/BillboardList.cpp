#include "BillboardList.h"
#include "OpenGlLayer.h"
#include "Texture.h"
#include "Renderer.h"
#include "Shader.h"
#include "ShaderProgram.h"
#include <vector>

BillboardList::BillboardList() :
	m_ShaderIndex(0),
	m_TextureIndex(INVALID_TEXTURE_LOCATION),
	m_NumInstances(0),
	m_VBO(0),
	m_VAO(0),
	m_BillboardScale(0)
{
}

BillboardList::~BillboardList()
{
	OpenGLLayer::clean_GL_vao(&this->m_VAO, 1);
	OpenGLLayer::clean_GL_buffer(&m_VBO, 1);
}

bool BillboardList::Init(size_t shaderIndex, size_t textureIndex, float scale, size_t numX, size_t numY, float displace, float offset, float yPos)
{
	m_ShaderIndex = shaderIndex;

	m_TextureIndex = textureIndex;
	m_BillboardScale = scale;

	m_NumInstances = numX * numY;

	// Hard code positions (for now), should allow them to set positions or randomly gen
	std::vector<Vec3> positions;

	for (unsigned int y = 0; y < numY; ++y)
	{
		for (unsigned int x = 0; x < numX; ++x)
		{
			positions.push_back(Vec3( ((float)x * displace) - offset, yPos, ((float)y * displace) - offset));
		}
	}

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * positions.size(), positions.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

bool BillboardList::InitWithPositions(size_t shaderIndex, size_t texture, float setScale, const std::vector<Vec3>& positions)
{
	m_ShaderIndex = shaderIndex;
	m_TextureIndex = texture;
	m_BillboardScale = setScale;

	m_NumInstances = positions.size();

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * positions.size(), positions.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

void BillboardList::SetScale(float scale)
{
	m_BillboardScale = Maths::Clamp(scale, 0.5f, 100.0f);
}

float BillboardList::GetScale() const
{
	return m_BillboardScale;
}