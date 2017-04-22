#ifndef __OUTDOOR_SCENE_H__
#define __OUTDOOR_SCENE_H__

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

class OutDoorScene : public IScene
{
public:
	OutDoorScene(const std::string& name);
	virtual ~OutDoorScene();

	int  OnSceneLoad(ResourceManager* resManager) override;
	void OnSceneExit() override;
	void Update(float dt) override;
	void Render() override;
	void RenderUI() override;

private:
	std::vector<GameObject*>	m_GameObjects;
	BillboardList*				m_TreeBillboardList;
	DirectionalLightC*			m_DirLightHandle;
	Transform*					m_GoblinTransform;
	Transform*					m_PistolTransform;
	Transform*					m_CamTransform;
	Animator*					m_GoblinAnim;
	TerrainConstructor*			m_TerrainConstructor;
	float						m_TimeNow;
	float						m_GoblinHeight;
};

#endif