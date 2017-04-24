#ifndef __SPACE_SCENE_H__
#define __SPACE_SCENE_H__


#include "IScene.h"
#include <vector>

class GameObject;
class BillboardList;
class Terrain;
class SurfaceMesh;
class DirectionalLightC;
class AnimMesh;
class Transform;
class Animator;
class TerrainConstructor;

class SpaceScene : public IScene
{
public:
	SpaceScene(const std::string& name);
	virtual ~SpaceScene();

	int  OnSceneLoad(ResourceManager* resManager) override;
	void OnSceneExit() override;
	void Update(float dt) override;
	void Render() override;
	void RenderUI() override;

private:
	std::vector<GameObject*>	m_GameObjects;
	DirectionalLightC*			m_DirLightHandle;
	Transform*					m_CamTransform;
	float						m_TimeNow;
};

#endif
