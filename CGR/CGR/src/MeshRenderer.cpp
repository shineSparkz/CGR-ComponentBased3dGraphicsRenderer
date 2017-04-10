#include "MeshRenderer.h"
#include "LogFile.h"

int MeshRenderer::m_Id = MESH_RENDER_COMPONENT;

MeshRenderer::MeshRenderer(GameObject* go) :
	Component(go)
{
}

MeshRenderer::~MeshRenderer()
{
}

void MeshRenderer::Start()
{
}

void MeshRenderer::Update()
{
}

void MeshRenderer::SetMesh(size_t meshIndex)
{
	this->MeshIndex = meshIndex;
}

void MeshRenderer::SetMaterialSet(size_t handle)
{
	m_MaterialIndex = handle;
}

void MeshRenderer::SetShader(size_t shader)
{
	m_ShaderIndex = shader;
}

void MeshRenderer::SetToUseBumpMaps(bool shouldUse)
{
	if (shouldUse)
		m_HasBumpMaps = GE_TRUE;
	else
		m_HasBumpMaps = GE_FALSE;
}

bool MeshRenderer::UsingBumpMaps() const
{
	return m_HasBumpMaps == GE_TRUE;
}