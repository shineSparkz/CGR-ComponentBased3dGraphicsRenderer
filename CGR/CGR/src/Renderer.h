#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "gl_headers.h"
#include "types.h"
#include "Lights.h"
#include "Colour.h"
#include "Vertex.h"
#include "FontAlign.h"
#include <vector>
#include <map>
#include "Singleton.h"//remove this and use evnts

class Mesh;
class MeshRenderer;
class LightTechnique;
class BasicDiffuseTechnique;
class BaseCamera;
class FlyCamera;
class Font;
class Texture;
class BillboardList;
class Terrain;
class GBuffer;
class GameObject;
class ShaderProgram;
class ShadowFrameBuffer;

// Get rid
class ShadowMapTechnique;
class LavaTechnique;
//ds
class GeometryPassTechnique;
class DSDirLightPassTech;
class DSPointLightPassTech;
class NullTechnique;

class Renderer : public Singleton<Renderer>
{
public:
	Renderer();
	bool Init();
	void Render();
	void Close();

	// Render With mesh resource, no textures or materials considered, use for deferred
	void RenderMesh(Mesh* mesh);

	void RenderMesh(MeshRenderer* mesh);
	void RenderSkybox(BaseCamera* cam);
	void RenderText(const std::string& txt, float x, float y, FontAlign fa = FontAlign::Left, const Colour& col = Colour::White());
	Texture* GetTexture(size_t index);
	void WindowSizeChanged(int w, int h);

private:
	// These will ALL be moved later, this is not dynamic or user enanled, but not concerned at this stage
	bool setRenderStates();
	bool setFrameBuffers();
	bool setLights();
	bool setCamera();
	bool loadFonts();
	bool loadTetxures();
	bool loadMeshes();
	bool createMaterials();

	bool loadTexture(const std::string& path, size_t key_store, int glTextureIndex);
	bool loadMesh(const std::string& path, size_t key_store, bool tangents);

	// Deferred
	void GeomPass();
	void StencilPass(int lightIndex);
	void PointLightPass(int lightIndex);
	void DirLightPass();
	void FinalPass();


private:
	// This will be somewhere else
	std::map<size_t, Texture*> m_Textures;
	std::map<size_t, Mesh*> m_Meshes;
	std::map<size_t, ShaderProgram*> m_Shaders;

	std::vector<GameObject*> m_GameObjects;

	Font* m_Font;
	GameObject* m_CameraObj;
	BaseCamera* m_Camera;//convenience
	GBuffer* m_Gbuffer;

	// SHould be a mesh renderer
	BillboardList* m_TreeBillboardList;
	Terrain* m_Terrain;

	// Deferred
	GeometryPassTechnique* m_GeomPassMaterial;
	DSPointLightPassTech* m_PointLightPassMaterial;
	DSDirLightPassTech* m_DirLightPassMaterial;
	NullTechnique* m_NullTech;

	// These need to be components of a game object
	DirectionalLight m_DirectionalLight;
	PointLight m_PointLights[3];
};


/*
class Renderer : public Singleton<Renderer>
{
public:
	Renderer();

	bool Init();
	void ShadowPass();
	void Render();
	void Close();
	void ReloadShaders();

	void RenderMesh(MeshRenderer* mesh, bool withTextures = true);
	void RenderSkybox(BaseCamera* cam);
	void RenderText(const std::string& txt, float x, float y, FontAlign fa = FontAlign::Left, const Colour& col = Colour::White() );
	
	Texture* GetTexture(size_t index);

	void WindowSizeChanged(int w, int h);

private:
	// These will ALL be moved later, this is not dynamic or user enanled, but not concerned at this stage
	bool setRenderStates();
	bool setFrameBuffers();
	bool setLights();
	bool setCamera();
	bool loadFonts();
	bool loadTetxures();
	bool loadMeshes();
	bool createMaterials();

	bool loadTexture(const std::string& path, size_t key_store, int glTextureIndex);
	bool loadMesh(const std::string& path, size_t key_store, bool tangents);

private:
	// Resources
	std::map<size_t, Texture*> m_Textures;
	std::map<size_t, Mesh*> m_Meshes;

	std::vector<GameObject*> m_GameObjects;

	MeshRenderer* m_LavaTestMesh;

	// Objects
	Font* m_Font;
	GameObject* m_CameaObj;
	FlyCamera* m_Camera;
	GameObject* m_LightCameaObj;
	BaseCamera* m_LightCamera;
	ShadowFrameBuffer* m_ShadowFBO;
	BillboardList* m_TreeBillboardList;
	Terrain* m_Terrain;

	// 'Materials'
	FontTechnique* m_FontMaterial;
	LightTechnique* m_LightMaterial;
	BasicDiffuseTechnique* m_DiffuseMaterial;
	SkyboxTechnique* m_SkyBoxMaterial;
	ShadowMapTechnique* m_ShadowMaterial;
	BillboardTechnique* m_BillboardMaterial;
	TerrainTechnique* m_TerrainMaterial;
	LavaTechnique* m_LavaMaterial;
	
	// Lights -- here for now
	DirectionalLight m_DirectionalLight;
	SpotLight m_SpotLights[1];
	//PointLight m_PointLights[2];

};
*/

#endif