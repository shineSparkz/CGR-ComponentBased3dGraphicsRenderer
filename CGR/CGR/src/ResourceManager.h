#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Singleton.h"
#include "Shader.h"
#include "Vertex.h"
#include <map>

struct Material;
class Texture;
class Mesh;
class AnimMesh;
class Font;
class UniformBlockManager;

class ResourceManager
{
public:
	// ---- Load Functions: Will be stored in this ----
	bool				LoadFont(const std::string& path, size_t key, int size);
	bool				LoadMesh(const std::string& path, size_t key_store, bool tangents, bool withTextures, unsigned materialSet);
	bool				LoadAnimMesh(const std::string& path, size_t key_store, unsigned materialSet, bool flipUvs);
	bool				CreateMesh(size_t key, const std::vector<Vertex>&, const std::vector<uint32>& indices, unsigned materialSet);	
	bool				LoadTexture(const std::string& path, size_t key_store, int glTextureIndex);
	bool				LoadCubeMap(std::string path[6], size_t key_store, int glTextureIndex);
	bool				CreateShaderProgram(std::vector<Shader>& shaders, size_t key);
	void				AddMaterialSet(size_t key, const std::map<unsigned, Material*> materials);
	
	// ---- Query Functions ----
	bool				CheckMeshExists(size_t key) const;
	bool				CheckAnimMeshExists(size_t key) const;
	bool				CheckTextureExists(size_t key) const;
	bool				CheckShaderExists(size_t key) const;
	bool				CheckFontExists(size_t key) const;
	bool				CheckMaterialSetExists(size_t key) const;

	// ---- Get Resource Funcitons ----
	ShaderProgram*		GetShader(size_t index) const;
	Texture*			GetTexture(size_t index) const;
	Mesh*				GetMesh(size_t index) const;
	AnimMesh*			GetAnimMesh(size_t index) const;
	Font*				GetFont(size_t index) const;

	// ---- Load Functions: Will NOT be stored in this and shoud be cleaned by caller ----
	ShaderProgram*		LoadAndGetShaderProgram(std::vector<Shader>& shaders);
	Texture*			LoadAndGetTexture(const std::string& textureFile, int textureSampler);
	Mesh*				LoadAndGetMesh(const std::string& path, bool tangents, bool withTextures, unsigned materialSet);
	Mesh*				CreateAndGetMesh(const std::vector<Vertex>& verts, const std::vector<uint32>& indices, unsigned materialSet);
	AnimMesh*			LoadAndGetAnimMesh(const std::string& path, unsigned materialSet, bool flipUvs);
	Font*				LoadAndGetFont(const std::string& path, int size);

private:
	bool				createDefaultResources();

	bool				loadDefaultTextures();
	bool				loadDefaultMeshes();
	bool				loadDefaultFonts();
	bool				loadDefaultForwardShaders();
	bool				loadDefaultDeferredShaders();

	void Close();

private:
	friend class Renderer;
	std::map<size_t, std::map<unsigned, Material*>>		m_Materials;
	std::map<size_t, ShaderProgram*>					m_Shaders;
	std::map<size_t, Texture*>							m_Textures;
	std::map<size_t, Mesh*>								m_Meshes;
	std::map<size_t, AnimMesh*>							m_AnimMeshes;
	std::map<size_t, Font*>								m_Fonts;
};

#endif