#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <vector>
#include <map>

#include "gl_headers.h"
#include "types.h"
#include "Time.h"
#include "Colour.h"
#include "FontAlign.h"
#include "Queery.h"
#include "EventHandler.h"

// Forward
class ResourceManager;
class Mesh;
class MeshRenderer;
class BaseCamera;
class Font;
class Texture;
class BillboardList;
class Terrain;
class GBuffer;
class GameObject;
class ShaderProgram;
class ShadowFrameBuffer;
class UniformBlockManager;



struct DeferredPointLightInfo
{
	Vec3 pos;
	float range;
};

enum ShadingMode
{
	Forward, Deferred
};

class Renderer : public EventHandler
{
public:
	Renderer();

	bool					Init();
	void					Close();
	bool					SetSceneData(BaseCamera* camera, const Vec3& ambientLight);

	// Public Rendering
	void					Render(std::vector<GameObject*>& gameObjects);
	void					RenderText(size_t fontId, const std::string& txt, float x, float y, FontAlign fa = FontAlign::Left, const Colour& col = Colour::White());
	void					RenderBillboardList(BillboardList* billboard);
	void					RenderTerrain(Terrain* terrain);

	// Get Resources
	size_t					GetNumSubMeshesInMesh(size_t meshIndex) const;
	size_t					GetNumTextures() const;
	Texture*				GetTexture(size_t index) const;
	ResourceManager* const	GetResourceManager() const;

	// Get Light info
	int						GetDirLightIndex();
	int						GetSpotLightIndex();
	int						GetPointLightIndex();
	void					UpdatePointLight(int index, const Vec3& position, float range);

	// Shading Mode
	void					ToggleShadingMode();
	void					SetShadingMode(ShadingMode mode);
	ShadingMode				GetShadingMode() const;
	std::string				GetShadingModeStr() const;

private:
	// Rendering
	void forwardRender(std::vector<GameObject*>& gameObjects);
	void deferredRender(std::vector<GameObject*>& gameObjects);
	void renderMesh(Mesh* mesh);
	void renderMesh(MeshRenderer* mesh);
	void renderSkybox(BaseCamera* cam);

	// Events
	void HandleEvent(Event* ev) override;
	void sceneChange();
	void windowSizeChanged(int w, int h);

	// Private init
	bool setFrameBuffers();
	bool setStaticDefaultShaderValues();
	bool createUniformBlocks();
	float getFrameTime(TimeMeasure tm);

private:
	UniformBlockManager*	m_UniformBlockManager;
	ResourceManager*		m_ResManager;
	BaseCamera*				m_CameraPtr;
	GBuffer*				m_Gbuffer;
	GLuint					m_QueryTime;
	uint64					m_Frames;
	Query					m_Query;

	int						m_NumDirLightsInScene;
	int						m_NumPointLightsInScene;
	int						m_NumSpotLightsInScene;

	ShadingMode				m_PendingShadingMode{ ShadingMode::Forward };
	ShadingMode				m_ShadingMode{ ShadingMode::Forward };
	bool					m_ShadingModePending{ false };

	std::vector<DeferredPointLightInfo>	m_PointsInfo;
};

INLINE ResourceManager* const Renderer::GetResourceManager() const
{
	return m_ResManager;
}


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