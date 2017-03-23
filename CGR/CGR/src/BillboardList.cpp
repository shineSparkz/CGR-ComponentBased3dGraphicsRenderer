#include "BillboardList.h"
#include "OpenGlLayer.h"
#include "Technique.h"
#include "Texture.h"
#include "Renderer.h"
#include "Shader.h"
#include <vector>

BillboardList::BillboardList() :
	m_Material(nullptr),
	m_TextureIndex(INVALID_TEXTURE_LOCATION),
	m_VBO(0)
{
}

BillboardList::~BillboardList()
{
	OpenGLLayer::clean_GL_vao(&this->m_VAO, 1);
	OpenGLLayer::clean_GL_buffer(&m_VBO, 1);
}

bool BillboardList::Init(ShaderProgram* mat, size_t textureIndex, float scale, size_t numX, size_t numY, float displace, float offset, float yPos)
{
	m_Material = mat;
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
	//glVertexAttribPointer(
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Should this really be here? Or just in material itself
	if (m_Material)
	{
		m_Material->Use();
		int sampler = 0;
		m_Material->SetUniformValue<int>("u_TextureMap", &sampler);
	}

	return true;
}

bool BillboardList::InitWithPositions(ShaderProgram* mat, size_t texture, float setScale, const std::vector<Vec3>& positions)
{
	m_Material = mat;
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
	//glVertexAttribPointer(
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Should this really be here? Or just in material itself
	if (m_Material)
	{
		m_Material->Use();
		int sampler = 0;
		m_Material->SetUniformValue<int>("u_TextureMap", &sampler);
	}

	return true;
}

void BillboardList::Render(Renderer* renderer, const Mat4& viewProj, const Vec3& camPos, const Vec3& camRight)
{
	// This should be set globally, not in this one billboard instance, 
	// what if we have a bunch of different billboards, god this guy is naff!
	m_Material->Use();

	m_Material->SetUniformValue<Mat4>("u_ViewProjXform", &viewProj);
	m_Material->SetUniformValue<Vec3>("u_CamPos", &camPos);
	m_Material->SetUniformValue<float>("u_Scale", &m_BillboardScale);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind the texture we get from renderer. This needs sorting too
	Texture* t = renderer->GetTexture(m_TextureIndex);
	if (t)t->Bind();

	// I think we shoulf be asking renderer to do this 
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_POINTS, 0, (GLsizei)m_NumInstances);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}