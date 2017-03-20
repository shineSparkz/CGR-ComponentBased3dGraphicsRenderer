#include "Mesh.h"

// assimp include files. These three are usually needed.
#include "assimp\Importer.hpp"	//OO version Header!
#include "assimp\postprocess.h"
#include "assimp\scene.h"
#include "assimp\DefaultLogger.hpp"
#include "assimp\LogStream.hpp"

#include <fstream>
#include "LogFile.h"
#include "Vertex.h"
#include "OpenGlLayer.h"
#include "Texture.h"

// This is no good having these global but doesn't seem to work otherwise!!
const struct aiScene* scene = NULL;
Assimp::Importer importer;

bool Import3DFromFile(const std::string& pFile)
{
	// Check if file exists
	std::ifstream fin(pFile.c_str());
	if (!fin.fail())
	{
		fin.close();
	}
	else
	{
		WRITE_LOG(importer.GetErrorString(), "error");
		return false;
	}

	scene = importer.ReadFile(pFile,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace);
		//aiProcess_JoinIdenticalVertices);

	// If the import failed, report it
	if (!scene)
	{
		WRITE_LOG(importer.GetErrorString(), "error");
		return false;
	}

	// Now we can access the file's contents.
	WRITE_LOG("Import of scene " + pFile + " succeeded.", "good");
	return true;
}


SubMesh::SubMesh() :
	NumIndices(0),
	NumVertices(0),
	BaseIndex(0),
	BaseVertex(0)
	//TextureIndices()
{
};

SubMesh::~SubMesh()
{
}

void SubMesh::Init(const aiMesh* paiMesh, std::vector<Vertex>& vertices, std::vector<dword>& indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute vectors
	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ?
			&(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		Vertex v;
		v.position = Vec3(pPos->x, pPos->y, pPos->z);
		v.normal = Vec3(pNormal->x, pNormal->y, pNormal->z);
		v.texcoord = Vec2(pTexCoord->x, pTexCoord->y);

		vertices.push_back(v);
	}

	// Populate the index buffer
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
	{
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		indices.push_back(Face.mIndices[0]);
		indices.push_back(Face.mIndices[1]);
		indices.push_back(Face.mIndices[2]);
	}
}

void SubMesh::Init(const aiMesh* paiMesh, std::vector<VertexTan>& vertices, std::vector<dword>& indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute vectors
	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
		const aiVector3D* pTangent = paiMesh->HasTangentsAndBitangents() ? &(paiMesh->mTangents[i]) : &Zero3D;

		VertexTan v;
		v.position = Vec3(pPos->x, pPos->y, pPos->z);
		v.normal = Vec3(pNormal->x, pNormal->y, pNormal->z);
		v.texcoord = Vec2(pTexCoord->x, pTexCoord->y);
		v.tangent = Vec3(pTangent->x, pTangent->y, pTangent->z);

		vertices.push_back(v);
	}

	// Populate the index buffer
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
	{
		const aiFace& Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		indices.push_back(Face.mIndices[0]);
		indices.push_back(Face.mIndices[1]);
		indices.push_back(Face.mIndices[2]);
	}
}


//--------------------------------
Mesh::Mesh() :
	m_IndexVBO(0),
	m_VAO(0),
	m_SubMeshes(),
	//m_TextureHandles(),
	m_VertexVBO(0)
{
}

Mesh::~Mesh()
{
	OpenGLLayer::clean_GL_vao(&this->m_VAO, 1);
	OpenGLLayer::clean_GL_buffer(&this->m_VertexVBO, 1);
	OpenGLLayer::clean_GL_buffer(&this->m_IndexVBO, 1);
}

bool Mesh::Load(const std::string& mesh, bool withTangents)
{
	// Will pass path here and have scene local
	if (!Import3DFromFile("../resources/meshes/" + mesh))
	{
		WRITE_LOG("Failed to load mesh", "error");
		return false;
	}

	// Create the VAO
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Create the buffers for the vertices atttributes
	glGenBuffers(1, &m_VertexVBO);
	glGenBuffers(1, &m_IndexVBO);

	// Load here
	m_SubMeshes.resize(scene->mNumMeshes);
	// Textures

	unsigned int num_vertices = 0;
	unsigned int num_indices = 0;

	// For one VAO for each mesh group, this all needs moving to the mesh class itself
	for (unsigned int i = 0; i < m_SubMeshes.size(); i++)
	{
		//mesh.m_MeshLayouts[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_SubMeshes[i].NumIndices = scene->mMeshes[i]->mNumFaces * 3;
		m_SubMeshes[i].BaseVertex = num_vertices;
		m_SubMeshes[i].BaseIndex = num_indices;

		num_vertices += scene->mMeshes[i]->mNumVertices;
		num_indices += m_SubMeshes[i].NumIndices;
	}
	
	std::vector<unsigned int> indices;
	indices.reserve(num_indices);

	if (!withTangents)
	{
		std::vector<Vertex> vertices;
		vertices.reserve(num_vertices);

		for (int m = 0; m < m_SubMeshes.size(); ++m)
		{
			m_SubMeshes[m].Init(scene->mMeshes[m], vertices, indices);
		}

		// Generate and populate the buffers with vertex attributes and the indices
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)24);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else
	{
		std::vector<VertexTan> vertTans;
		vertTans.reserve(num_vertices);

		for (int m = 0; m < m_SubMeshes.size(); ++m)
		{
			m_SubMeshes[m].Init(scene->mMeshes[m], vertTans, indices);
		}

		// Generate and populate the buffers with vertex attributes and the indices
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexTan) * vertTans.size(), vertTans.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTan), 0);
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTan), (void*)12);
		
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTan), (void*)24);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTan), (void*)32);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// End
	glBindVertexArray(0);

	//InitMaterials();
	return true;
}

/*
bool Mesh::AddTexture(size_t texHandle, unsigned int meshIndex)
{
	if (meshIndex >= m_MeshLayouts.size())
	{
		WRITE_LOG("Tried to load texture with out of bounds mesh index", "error");
		return false;
	}

	//m_MeshLayouts[meshIndex].TextureIndex = m_TextureHandles.size();
	m_MeshLayouts[meshIndex].TextureIndices.push_back(m_TextureHandles.size());

	m_TextureHandles.push_back(texHandle);
	return true;
}

bool Mesh::AddTexture(size_t texHandle)
{
	for (auto m = m_MeshLayouts.begin(); m != m_MeshLayouts.end(); ++m)
	{
		//m->TextureIndex = m_TextureHandles.size();
		m->TextureIndices.push_back(m_TextureHandles.size());
	}

	m_TextureHandles.push_back(texHandle);
	return true;
}
*/