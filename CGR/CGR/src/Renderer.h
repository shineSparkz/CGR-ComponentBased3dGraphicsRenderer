#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Time.h"
#include "gl_headers.h"
#include "types.h"
#include "Lights.h"
#include "Colour.h"
#include "Vertex.h"
#include "FontAlign.h"
#include <vector>
#include <map>
#include "Queery.h"
#include "Singleton.h"//remove this and use evnts

class ResourceManager;
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
class UniformBlockManager;

// Get rid
class ShadowMapTechnique;

class Renderer : public Singleton<Renderer>
{
public:
	Renderer();

	bool Init();
	void SceneChange();
	bool SetSceneData(BaseCamera* camera, const Vec3& ambientLight);
	void Render(std::vector<GameObject*>& gameObjects);
	void RenderText(size_t fontId, const std::string& txt, float x, float y, FontAlign fa = FontAlign::Left, const Colour& col = Colour::White());
	void WindowSizeChanged(int w, int h);
	void Close();
	
	size_t GetNumSubMeshesInMesh(size_t meshIndex) const;
	size_t GetNumTextures() const;
	Texture* GetTexture(size_t index) const;

	int GetDirLightIndex();
	int GetSpotLightIndex();
	int GetPointLightIndex();

private:
	void forwardRender(std::vector<GameObject*>& gameObjects);
	void deferredRender(std::vector<GameObject*>& gameObjects);
	void renderMesh(Mesh* mesh);
	void renderMesh(MeshRenderer* mesh);
	void renderSkybox(BaseCamera* cam);
	bool setFrameBuffers();
	bool setStaticDefaultShaderValues();
	float getFrameTime(TimeMeasure tm);

	bool createUniformBlocks();

private:
	UniformBlockManager*	m_UniformBlockManager;
	ResourceManager*		m_ResManager;
	BaseCamera*				m_CameraPtr;
	GBuffer*				m_Gbuffer;
	GLuint					m_QueryTime;
	uint64					m_Frames;
	Query					m_Query;

	// Remove these
	BillboardList*			m_TreeBillboardList;
	Terrain*				m_Terrain;

	int						m_NumDirLightsInScene;
	int						m_NumPointLightsInScene;
	int						m_NumSpotLightsInScene;
	bool					m_DeferredRender{ true };
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