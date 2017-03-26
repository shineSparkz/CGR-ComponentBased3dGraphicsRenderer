#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Singleton.h"
#include "Shader.h"
#include <map>

class Texture;
class Mesh;
class Font;

class ResourceManager : public Singleton<ResourceManager>
{
public:
	bool CreateDefaultResources();
	bool LoadFont(const std::string& path, size_t key, int size);
	bool LoadMesh(const std::string& path, size_t key_store, bool tangents, bool withTextures);
	bool LoadTexture(const std::string& path, size_t key_store, int glTextureIndex);
	bool LoadCubeMap(std::string path[6], size_t key_store, int glTextureIndex);
	bool CreateShaderProgram(std::vector<Shader>& shaders, size_t key);

	size_t GetNumSubMeshesInMesh(size_t meshIndex) const;
	size_t GetNumTextures() const;
	Texture* GetTexture(size_t index);
	ShaderProgram* GetShader(size_t index);

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
};

#endif