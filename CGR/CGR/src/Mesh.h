#ifndef __MESH_H__
#define __MESH_H__

#include <string>
#include <vector>

#include "gl_headers.h"
#include "types.h"
#include "Vertex.h"

class Image;
class Texture;
struct aiMesh;

struct MeshLayout
{
	MeshLayout();
	~MeshLayout();

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
	bool AddTexture(Image* i, int activeTexture, unsigned int meshIndex);
	bool AddTexture(Image* i, int activeTexture);

	//private:
	std::vector<MeshLayout> m_MeshLayouts;
	std::vector<Texture*> m_Textures;
	GLuint m_VAO;
	GLuint m_VertexVBO;
	GLuint m_IndexVBO;
};

#endif
