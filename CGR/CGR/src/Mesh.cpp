#include "Mesh.h"

#include <fstream>
#include "LogFile.h"
#include "Vertex.h"
#include "OpenGlLayer.h"
#include "Texture.h"
#include "Material.h"
#include "Image.h"
#include "ResourceManager.h"

// Assimp
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"
#include "assimp\DefaultLogger.hpp"
#include "assimp\LogStream.hpp"

uint64 Mesh::NumVerts = 0;
uint64 Mesh::NumMeshes = 0;

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
		aiProcess_CalcTangentSpace
		//| aiProcess_FindInstances
		);
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
{
};

SubMesh::~SubMesh()
{
}

void SubMesh::Init(const aiMesh* paiMesh, std::vector<Vertex>& vertices, std::vector<dword>& indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	Vec3 tempMin(MAX_TYPE(float));
	Vec3 tempMax(-MAX_TYPE(float));

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

		if ((v.position.x < tempMin.x))
		{
			tempMin.x = v.position.x;
		}
		if ((v.position.y < tempMin.y))
		{
			tempMin.y = v.position.y;
		}
		if ((v.position.z < tempMin.z))
		{
			tempMin.z = v.position.z;
		}

		if ((v.position.x > tempMax.x))
		{
			tempMax.x = v.position.x;
		}
		if ((v.position.y > tempMax.y))
		{
			tempMax.y = v.position.y;
		}
		if ((v.position.z > tempMax.z))
		{
			tempMax.z = v.position.z;
		}

		vertices.push_back(v);
	}

	minvertex = tempMin;
	maxVertex = tempMax;
	centre = (minvertex + maxVertex) / 2.0f;

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

	Vec3 tempMin(MAX_TYPE(float));
	Vec3 tempMax(-MAX_TYPE(float));

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

		if ((v.position.x < tempMin.x))
		{
			tempMin.x = v.position.x;
		}
		if ((v.position.y < tempMin.y))
		{
			tempMin.y = v.position.y;
		}
		if ((v.position.z < tempMin.z))
		{
			tempMin.z = v.position.z;
		}

		if ((v.position.x > tempMax.x))
		{
			tempMax.x = v.position.x;
		}
		if ((v.position.y > tempMax.y))
		{
			tempMax.y = v.position.y;
		}
		if ((v.position.z > tempMax.z))
		{
			tempMax.z = v.position.z;
		}

		vertices.push_back(v);
	}

	minvertex = tempMin;
	maxVertex = tempMax;
	centre = (minvertex + maxVertex) / 2.0f;

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
	m_VertexVBO(0)
{
}

Mesh::~Mesh()
{
	OpenGLLayer::clean_GL_vao(&this->m_VAO, 1);
	OpenGLLayer::clean_GL_buffer(&this->m_VertexVBO, 1);
	OpenGLLayer::clean_GL_buffer(&this->m_IndexVBO, 1);
}

bool Mesh::Load(const std::string& mesh, bool withTangents, bool loadTextures, unsigned textureSet, ResourceManager* resMan)
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

	// Static data
	Mesh::NumMeshes += scene->mNumMeshes;

	unsigned int num_vertices = 0;
	unsigned int num_indices = 0;

	// For one VAO for each mesh group, this all needs moving to the mesh class itself
	for (unsigned int i = 0; i < m_SubMeshes.size(); i++)
	{
		if (loadTextures)
		{
			m_SubMeshes[i].MaterialIndex = scene->mMeshes[i]->mMaterialIndex;
		}
		else
		{
			m_SubMeshes[i].MaterialIndex = i;
		}
		
		m_SubMeshes[i].NumIndices = scene->mMeshes[i]->mNumFaces * 3;
		m_SubMeshes[i].BaseVertex = num_vertices;
		m_SubMeshes[i].BaseIndex = num_indices;

		num_vertices += scene->mMeshes[i]->mNumVertices;
		num_indices += m_SubMeshes[i].NumIndices;
	}

	Mesh::NumVerts += num_vertices;
	
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

	if (loadTextures)
	{
		//m_Materials.resize(scene->mNumMaterials);
		return InitMaterials(scene, mesh, textureSet, resMan);
	}

	return true;
}

bool Mesh::InitMaterials(const aiScene* pScene, const std::string& filename, unsigned textureSet, ResourceManager* resMan)
{
	std::map<unsigned, Material*> materials;

	// Extract the directory part from the file name
	std::string::size_type slashIndex = filename.find_last_of("/");
	std::string dir;

	bool return_value = true;

	if (slashIndex == std::string::npos)
	{
		dir = ".";
	}
	else if (slashIndex == 0)
	{
		dir = "/";
	}
	else
	{
		dir = filename.substr(0, slashIndex);
	}

	// Hack I had to put in for daft cunts that put spaces in directory paths
	bool didWarn = false;

	// Initialize the materials
	for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		materials[i] = nullptr;

		/*
		{
			unsigned diff_count = 0;
			unsigned spec_count = 0;
			unsigned amb_count = 0;
			unsigned emis_count = 0;
			unsigned height_count = 0;
			unsigned norm_count = 0;
			unsigned shine_count = 0;
			unsigned opac_count = 0;
			unsigned disp_count = 0;
			unsigned lightmap_count = 0;
			unsigned reflect_count = 0;
			unsigned unknown_count = 0;

			diff_count += pMaterial->GetTextureCount(aiTextureType_DIFFUSE);
			spec_count += pMaterial->GetTextureCount(aiTextureType_SPECULAR);
			amb_count += pMaterial->GetTextureCount(aiTextureType_AMBIENT);
			emis_count += pMaterial->GetTextureCount(aiTextureType_EMISSIVE);
			height_count += pMaterial->GetTextureCount(aiTextureType_HEIGHT);
			norm_count += pMaterial->GetTextureCount(aiTextureType_NORMALS);
			shine_count += pMaterial->GetTextureCount(aiTextureType_SHININESS);
			opac_count += pMaterial->GetTextureCount(aiTextureType_OPACITY);
			disp_count += pMaterial->GetTextureCount(aiTextureType_DISPLACEMENT);
			lightmap_count += pMaterial->GetTextureCount(aiTextureType_LIGHTMAP);
			reflect_count += pMaterial->GetTextureCount(aiTextureType_REFLECTION);
			unknown_count += pMaterial->GetTextureCount(aiTextureType_UNKNOWN);
		}
		*/

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string p(path.data);

				if (p.substr(0, 2) == ".\\")
				{
					p = p.substr(2, p.size() - 2);
				}

				std::string fullPath = "../resources/meshes/" + dir + "/" + p;

				for (int i = 0; i < fullPath.length(); ++i)
				{
					if (fullPath[i] == ' ')
					{
						if (!didWarn)
						{
							WRITE_LOG("NEED TO REPLACE SPACES IN PATH WITH _ for mesh " + fullPath, "warning");
							didWarn = true;
						}

						fullPath[i] = '_';
					}
				}

				Image img;
				if (!img.LoadImg(fullPath.c_str()))
				{
					return_value = false;
					break;
				}

				if(!materials[i])
					materials[i] = new Material();

				materials[i]->diffuse_map = new Texture(fullPath, GL_TEXTURE_2D, GL_TEXTURE0);
				
				if (!materials[i]->diffuse_map->Create(&img))
				{
					return_value = false;
					break;
				}
			}
		}
		else
		{
			// Use Error texture
			std::string fullPath = "../resources/textures/error.tga";

			Image img;
			if (!img.LoadImg(fullPath.c_str()))
			{
				return_value = false;
				break;
			}

			if (!materials[i])
				materials[i] = new Material;

			materials[i]->diffuse_map = new Texture(fullPath, GL_TEXTURE_2D, GL_TEXTURE0);
			
			if (!materials[i]->diffuse_map->Create(&img))
			{
				return_value = false;
			}
		}
		
		if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string p(path.data);

				if (p.substr(0, 2) == ".\\")
				{
					p = p.substr(2, p.size() - 2);
				}

				std::string fullPath = "../resources/meshes/" + dir + "/" + p;

				for (int i = 0; i < fullPath.length(); ++i)
				{
					if (fullPath[i] == ' ')
					{
						if (!didWarn)
						{
							WRITE_LOG("NEED TO REPLACE SPACES IN PATH WITH _ for mesh " + fullPath, "warning");
							didWarn = true;
						}

						fullPath[i] = '_';
					}
				}

				Image img;
				if (!img.LoadImg(fullPath.c_str()))
				{
					return_value = false;
					break;
				}

				if (!materials[i])
					materials[i] = new Material();

				materials[i]->normal_map = new Texture(fullPath, GL_TEXTURE_2D, GL_TEXTURE2);

				if (!materials[i]->normal_map->Create(&img))
				{
					return_value = false;
					break;
				}
			}
		}
		else
		{
			// Use fake normal
			std::string fullPath = "../resources/textures/default_normal_map.tga";

			Image img;
			if (!img.LoadImg(fullPath.c_str()))
			{
				return_value = false;
				break;
			}

			if (!materials[i])
				materials[i] = new Material;

			materials[i]->normal_map = new Texture(fullPath, GL_TEXTURE_2D, GL_TEXTURE2);
			if (!materials[i]->normal_map->Create(&img))
			{
				return_value = false;
				break;
			}
		}
	}

	if (!return_value)
	{
		// Clean
		for (auto i = materials.begin(); i != materials.end(); ++i)
		{
			i->second->Clean();
			SAFE_DELETE(i->second);
		}

		materials.clear();
	}
	else
	{
		resMan->AddMaterialSet(textureSet, materials);
	}

	return return_value;
}