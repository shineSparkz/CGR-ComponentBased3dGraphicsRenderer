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
	virtual	~MeshRenderer();

	void Start() override;
	void Update() override;

	void SetMesh(size_t meshIndex);
	void SetShader(size_t texture);
	void SetMaterialSet(size_t handle);

	void SetToUseBumpMaps(bool shouldUse);
	bool UsingBumpMaps() const;

	// Temp
	bool ReceiveShadows = false;
	bool MultiTextures = false;

private:
	friend class						Renderer;
	static int							m_Id;
	size_t								MeshIndex;
	size_t								m_MaterialIndex;
	size_t								m_ShaderIndex;
	int									m_HasBumpMaps{ GE_FALSE };
};

INLINE int MeshRenderer::GetId()
{
	return m_Id;
}


#endif