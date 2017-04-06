#ifndef __INDOOR_LEVEL_SCENE_H__
#define __INDOOR_LEVEL_SCENE_H__

#include <vector>
#include "IScene.h"

class GameObject;
class BillboardList;

class IndoorLevelScene: public IScene
{
public:
	IndoorLevelScene(const std::string& name);
	virtual ~IndoorLevelScene();

	virtual int  OnSceneCreate() override;
	virtual int  OnSceneLoad() override;
	virtual void OnSceneExit() override;
	virtual void Update(float dt) override;
	virtual void Render(Renderer* renderer) override;

private:
	std::vector<GameObject*>	m_GameObjects;
	BillboardList*				m_TreeBillboardList;
};

#endif