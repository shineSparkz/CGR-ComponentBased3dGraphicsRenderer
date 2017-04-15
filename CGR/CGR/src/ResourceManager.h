#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Singleton.h"
#include "Shader.h"
#include "Vertex.h"
#include <map>

struct Material;
class Texture;
class Mesh;
class Font;
class UniformBlockManager;

class ResourceManager
{
public:
	bool LoadFont(const std::string& path, size_t key, int size);
	bool LoadMesh(const std::string& path, size_t key_store, bool tangents, bool withTextures, unsigned materialSet);
	bool CreateMesh(size_t key, const std::vector<Vertex>&, const std::vector<uint32>& indices, unsigned materialSet);
	bool LoadTexture(const std::string& path, size_t key_store, int glTextureIndex);
	bool LoadCubeMap(std::string path[6], size_t key_store, int glTextureIndex);
	bool CreateShaderProgram(std::vector<Shader>& shaders, size_t key);

	bool CheckMeshExists(size_t key);
	bool CheckTextureExists(size_t key);
	bool CheckShaderExists(size_t key);
	bool CheckFontExists(size_t key);

	void AddMaterialSet(size_t key, const std::map<unsigned, Material*> materials);
	bool MaterialSetExists(size_t key) const;

	size_t GetNumSubMeshesInMesh(size_t meshIndex) const;
	size_t GetNumTextures() const;
	Texture* GetTexture(size_t index);
	ShaderProgram* GetShader(size_t index);
	Mesh* GetMesh(size_t index) const;

	Texture* LoadTexture(const std::string& textureFile, int textureSampler);
private:

private:
	bool CreateDefaultResources();

private:
	bool loadDefaultTextures();
	bool loadDefaultMeshes();
	bool loadDefaultFonts();
	bool loadDefaultForwardShaders();
	bool loadDefaultDeferredShaders();

	void Close();

private:
	friend class Renderer;
	std::map<size_t, Texture*> m_Textures;
	std::map<size_t, Mesh*> m_Meshes;
	std::map<size_t, ShaderProgram*> m_Shaders;
	std::map<size_t, Font*> m_Fonts;

	std::map<size_t, std::map<unsigned, Material*>> m_Materials;
};

#endif