#ifndef __MESH_RENDERER_H__
#define __MESH_RENDERER_H__

#include "types.h"
#include "Component.h"
#include <vector>

class GameObject;

class MeshRenderer : public Component
{
public:
	MeshRenderer(GameObject* go);
	static int GetId();
	virtual ~MeshRenderer();

	void Start() override;
	void Update() override;

	void SetMesh(size_t meshIndex, size_t numSubMeshes);
	bool AddTexture(size_t texHandle, size_t meshIndex);
	bool AddTexture(size_t texHandle);

	size_t m_ShaderIndex = 0;

private:
	friend class Renderer;
	static int m_Id;

	// Key for resource in hash table
	size_t MeshIndex;

	// Keys for textures in resource hash table
	std::vector<size_t> m_TextureHandles;
	std::vector<std::vector<size_t>> m_SubMeshTextures;
};

INLINE int MeshRenderer::GetId()
{
	return m_Id;
}


#endif