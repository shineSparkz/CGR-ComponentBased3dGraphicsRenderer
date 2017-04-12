#ifndef __OUTDOOR_SCENE_H__
#define __OUTDOOR_SCENE_H__

#include "IScene.h"
#include <vector>

class GameObject;
class BillboardList;
class Terrain;
class SurfaceMesh;
class DirectionalLightC;

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
	SurfaceMesh*				m_Surface[2];
	BillboardList*				m_TreeBillboardList;

	DirectionalLightC*			m_DirLightHandle;
	float						m_TimeNow;
	bool						m_WireFrame{ !false };
	bool						m_RenderSurface{ !false };
};

#endif