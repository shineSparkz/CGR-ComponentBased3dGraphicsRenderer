#ifndef __OUTDOOR_SCENE_H__
#define __OUTDOOR_SCENE_H__

#include "IScene.h"
#include <vector>

class GameObject;
class BillboardList;
class Terrain;

class OutDoorScene : public IScene
{
public:
	OutDoorScene(const std::string& name);
	virtual ~OutDoorScene();

	virtual int  OnSceneLoad(ResourceManager* resManager) override;
	virtual void OnSceneExit() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

private:
	std::vector<GameObject*>	m_GameObjects;
	Terrain*					m_Terrain;
	BillboardList*				m_TreeBillboardList;
};

#endif