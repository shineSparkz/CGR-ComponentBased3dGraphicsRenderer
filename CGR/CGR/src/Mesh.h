#ifndef __MESH_H__
#define __MESH_H__

#include <string>
#include <vector>

#include "gl_headers.h"
#include "types.h"
#include "Vertex.h"

struct aiMesh;

struct SubMesh
{
	SubMesh();
	~SubMesh();

	void Init(const aiMesh* paiMesh, std::vector<Vertex>& vertices, std::vector<dword>& indices);

	unsigned int NumIndices;
	unsigned int NumVertices;
	unsigned int TextureIndex;
	int BaseVertex;
	int BaseIndex;
};

class Mesh
{
public:
	~Mesh();

	bool Load(const std::string& mesh);
	
	bool AddTexture(size_t texHandle, unsigned int meshIndex);
	bool AddTexture(size_t texHandle);

	//private:
	std::vector<SubMesh> m_MeshLayouts;
	std::vector<size_t> m_TextureHandles;
	GLuint m_VAO;
	GLuint m_VertexVBO;
	GLuint m_IndexVBO;
};

#endif
