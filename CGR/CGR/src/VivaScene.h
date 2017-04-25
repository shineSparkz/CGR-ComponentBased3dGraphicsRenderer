#ifndef __VIVA_SCENE_H__
#define __VIVA_SCENE_H__

#include "IScene.h"
#include <vector>
#include <string>

class GameObject;
class BillboardList;
class Terrain;
class SurfaceMesh;
class DirectionalLightC;
class AnimMesh;
class Transform;
class Animator;
class TerrainConstructor;

class VivaScene : public IScene
{
public:
	VivaScene(const std::string& name);
	virtual ~VivaScene();

	int  OnSceneLoad(ResourceManager* resManager) override;
	void OnSceneExit() override;
	void Update(float dt) override;
	void Render() override;
	void RenderUI() override;

private:
	std::vector<GameObject*>	m_GameObjects;
	Transform*					m_CamTransform;
	TerrainConstructor*			m_Terrain;
	float						m_TimeNow;
};

#endif