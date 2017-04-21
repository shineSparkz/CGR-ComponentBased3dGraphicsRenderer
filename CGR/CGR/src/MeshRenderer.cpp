#include "MeshRenderer.h"
#include "LogFile.h"

int MeshRenderer::m_Id = MESH_RENDER_COMPONENT;

MeshRenderer::MeshRenderer(GameObject* go) :
	Component(go),
	m_MeshIndex(0),
	m_MaterialIndex(0),
	m_ShaderIndex(0),
	m_HasBumpMaps(GE_FALSE),
	m_ReceiveShadows(GE_FALSE),
	m_MultiTextures(false),
	m_HasAnimations(false)
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

void MeshRenderer::SetMeshData(
	size_t	meshResourceIndex,
	size_t	shaderProgramIndex,
	size_t	materialSetIndex,
	bool	useBumpMaps,
	bool	receiveShadows,
	bool	hasMultiTextures,
	bool	isAnimatedMesh)
{
	m_MeshIndex = meshResourceIndex;
	m_ShaderIndex = shaderProgramIndex;
	m_MaterialIndex = materialSetIndex;
	m_HasBumpMaps = useBumpMaps ? GE_TRUE : GE_FALSE;
	m_ReceiveShadows = receiveShadows ? GE_TRUE : GE_FALSE;
	m_MultiTextures = hasMultiTextures;
	m_HasAnimations = isAnimatedMesh;
}

bool MeshRenderer::UsingBumpMaps() const
{
	return m_HasBumpMaps == GE_TRUE;
}

bool MeshRenderer::ReceivingShadows() const
{
	return m_ReceiveShadows == GE_TRUE;
}

bool MeshRenderer::HasAnimations() const
{
	return m_HasAnimations;
}

bool MeshRenderer::HasMultiTextures() const
{
	return m_MultiTextures;
}

void MeshRenderer::SetUseBumpMaps(bool should)
{
	m_HasBumpMaps = should ? GE_TRUE : GE_FALSE;
}