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

void MeshRenderer::SetMesh(size_t meshIndex, size_t numSubMeshes)
{
	this->MeshIndex = meshIndex;
	m_SubMeshTextures.resize(numSubMeshes);
}

bool MeshRenderer::AddTexture(size_t texHandle, size_t meshIndex)
{
	if (meshIndex >= m_SubMeshTextures.size())
	{
		WRITE_LOG("Tried to load texture with out of bounds mesh index", "error");
		return false;
	}

	m_SubMeshTextures[meshIndex].push_back(m_TextureHandles.size());

	m_TextureHandles.push_back(texHandle);

	return true;
}

bool MeshRenderer::AddTexture(size_t texHandle)
{
	for (auto m = m_SubMeshTextures.begin(); m != m_SubMeshTextures.end(); ++m)
	{
		m->push_back(m_TextureHandles.size());
	}

	m_TextureHandles.push_back(texHandle);
	return true;
}