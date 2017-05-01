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
	virtual	~MeshRenderer();
	
	static int GetId();

	void Start() override;
	void Update() override;

	/*
	@ param - meshResourceIndex		:  The resource index of mesh to use from res manager, should use animated mesh if 'isAnimmated' flag set to true
	@ param - shaderProgramIndex	:  The resource index of the loaded shader program to be used by this mesh, note* must follow certain rules
	@ param - materialSetIndex		:  The resource index of the material used by this mesh
	@ param - useBumpMaps			:  Set to true if you wish to use bump maps, the material must have a normal map texture
	@ param - receiveShadows		:  Set to true if you want the mesh to receive shadows, glag must be set in the render function of class Renderer for this to work
	@ param - hasMultiTextures		:  Set to true for meshes with multiple diffuse textures, such as terrain, this will ensure they are all bound and not just individual to the sub mesh
	@ param - isAnimatedMesh		:  Set to true if you have correctly indexed an animated mesh resource
	*/
	void SetMeshData(
		size_t	meshResourceIndex,
		size_t	shaderProgramIndex,
		size_t	materialSetIndex,
		bool	useBumpMaps,
		bool	receiveShadows,
		bool	hasMultiTextures,
		bool	isAnimatedMesh
	);

	bool UsingBumpMaps() const;
	bool ReceivingShadows() const;
	bool HasAnimations() const;
	bool HasMultiTextures() const;

	void SetUseBumpMaps(bool should);
	void SetReceiveShadows(bool should);
	void SetMaterialIndex(size_t index);
	void SetShaderIndex(size_t index);
	void SetMeshIndex(size_t index);

private:
	friend class						Renderer;
	static int							m_Id;

	size_t								m_MeshIndex;
	size_t								m_MaterialIndex;
	size_t								m_ShaderIndex;
	int									m_HasBumpMaps{ GE_FALSE };
	int									m_ReceiveShadows{ GE_FALSE };
	bool								m_MultiTextures{ false };
	bool								m_HasAnimations{ false };
};

INLINE int MeshRenderer::GetId()
{
	return m_Id;
}


#endif