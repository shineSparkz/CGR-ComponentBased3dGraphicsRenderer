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
class GBuffer;
class GameObject;
class ShaderProgram;
class ShadowFrameBuffer;
class UniformBlockManager;
class Frustum;
class AnimMesh;
class Animator;

struct DeferredPointLightInfo
{
	Vec3 pos;
	float range;
};

enum ShadingMode
{
	Forward, Deferred
};

enum PolygonMode
{
	Filled,
	WireFrame
};

class Renderer : public EventHandler
{
public:
	Renderer();

	bool					Init();
	void					Close();
	bool					SetSceneData(BaseCamera* camera, const Vec3& ambientLight);
	const std::string&		GetHardwareStr() const;

	// Public Rendering
	void					Render(std::vector<GameObject*>& gameObjects, bool withShadows = false);
	void					RenderText(size_t fontId, const std::string& txt, float x, float y, FontAlign fa = FontAlign::Left, const Colour& col = Colour::White());
	void					RenderBillboardList(BillboardList* billboard);

	// Reload
	bool					ReloadShaders();

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

	// Modes
	void					ToggleShadingMode();
	void					SetShadingMode(ShadingMode mode);
	ShadingMode				GetShadingMode() const;
	std::string				GetShadingModeStr() const;

	std::string				GetPolygonModeStr() const;
	void					TogglePolygonMode();
	void					SetPolygonMode(PolygonMode mode);
	void					DisplayNormals(bool shouldDisplay);
	bool					IsDisplayingNormals() const;

	bool					IsQueeryingFrames() const;
	void					ToggleFrameQueeryMode();

	void					ToggleFrustumCulling();
	void					SetFrustumCulling(bool should);

	void					ToggleDisplayInfo();
	void					SetDisplayInfo(bool should);

private:
	// Rendering
	void forwardRenderShadows(std::vector<GameObject*>& gameObjects);
	void forwardRender(std::vector<GameObject*>& gameObjects, bool withShadows = false);
	void deferredRender(std::vector<GameObject*>& gameObjects);
	void renderMesh(Mesh* mesh);
	void renderMesh(MeshRenderer* mesh, const Mat4& world, bool withTextures, GLenum renderMode = GL_TRIANGLES);
	void renderAnimMesh(MeshRenderer* mesh, Animator* anim, const Mat4& world, bool withTextures);
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
	std::vector<DeferredPointLightInfo>		m_PointsInfo;
	UniformBlockManager*					m_UniformBlockManager;
	ResourceManager*						m_ResManager;
	BaseCamera*								m_CameraPtr;
	GBuffer*								m_Gbuffer;
	ShadowFrameBuffer*						m_ShadowFB;
	Frustum*								m_Frustum;
	GameObject*								m_LightCamObj;
	BaseCamera*								m_LightCamera;
	GLuint									m_QueryTime;
	Query									m_Query;
	std::string								m_HardwareStr;

	int										m_NumDirLightsInScene;
	int										m_NumPointLightsInScene;
	int										m_NumSpotLightsInScene;

	PolygonMode								m_PolyMode{ PolygonMode::Filled };
	ShadingMode								m_PendingShadingMode{ ShadingMode::Forward };
	ShadingMode								m_ShadingMode{ ShadingMode::Forward };
	bool									m_ShadingModePending{ false };

	bool									m_ShouldDisplayNormals{ false };
	bool									m_ShouldQueryFrames{ false };
	bool									m_ShouldFrustumCull{ !true };
	bool									m_ShouldDisplayInfo{ true };
	int										m_CullCount = 0;

};

INLINE ResourceManager* const Renderer::GetResourceManager() const
{
	return m_ResManager;
}

#endif