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
class LightTechnique;
class BasicDiffuseTechnique;
class Camera;
class Font;
class Texture;
class BillboardList;
class Terrain;

class FontTechnique;
class SkyboxTechnique;
class ShadowFrameBuffer;
class ShadowMapTechnique;
class BillboardTechnique;
class TerrainTechnique;
class LavaTechnique;

class Renderer : public Singleton<Renderer>
{
public:
	Renderer();

	bool Init();
	void ShadowPass();
	void Render();
	void Close();
	void ReloadShaders();

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

private:
	// Resources
	std::map<size_t, Texture*> m_Textures;
	std::vector<Mesh*> m_Meshes;	// these are mesh instances, not actual mesh resources, needs refactor
	Mesh* m_SkyboxMesh;

	Mesh* m_LavaTestMesh;

	// Objects
	Font* m_Font;
	Camera* m_Camera;
	Camera* m_LightCamera;
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

#endif