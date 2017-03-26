#ifndef __MESH_H__
#define __MESH_H__

#include <string>
#include <vector>

#include "gl_headers.h"
#include "types.h"
#include "Vertex.h"

#include "Texture.h"

struct aiMesh;
struct aiScene;

struct SubMesh
{
	SubMesh();
	~SubMesh();

	void Init(const aiMesh* paiMesh, std::vector<Vertex>& vertices, std::vector<dword>& indices);
	void Init(const aiMesh* paiMesh, std::vector<VertexTan>& vertices, std::vector<dword>& indices);

	unsigned int NumIndices;
	unsigned int NumVertices;
	unsigned int MaterialIndex;	// Should be an array for multiple textures (later)
	int BaseVertex;
	int BaseIndex;
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	static uint64 NumVerts;
	static uint64 NumMeshes;

	/*
		@param: mesh -- Full Path of mesh resource you wish to load
		@param: withTangents -- If you specify true, then this data will be sent to the GPU, it is epected that this is handled in the
								shader used to render the mesh, such as bump mapping
		@param: loadTextures -- Set to true if you only plan to use one of this mesh, and you know the mesh file contains valid materuial info.
								If this is false, then it is expected that you set the texture(s) on the MeshRenderer component, if true
								then the file must contain material information, currently only supports one texture per sub-mesh, that is
								diffuse. If the texture does not exist then a pink error texture will be used from the engine
	*/
	bool Load(const std::string& mesh, bool withTangents, bool loadTextures);

	bool InitMaterials(const aiScene* pScene, const std::string& filename);

	bool HasTextures() const
	{
		return m_Textures.size() > 0;
	}

	size_t GetNumSubMeshes() const;
	
private:
	friend class Renderer;
	std::vector<SubMesh> m_SubMeshes;
	GLuint m_VAO;
	GLuint m_VertexVBO;
	GLuint m_IndexVBO;

	std::vector<Texture*> m_Textures;
};

INLINE size_t Mesh::GetNumSubMeshes() const
{
	return m_SubMeshes.size();
}
#endif
