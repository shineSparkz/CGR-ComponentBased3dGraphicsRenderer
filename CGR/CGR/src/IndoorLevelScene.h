#ifndef __INDOOR_LEVEL_SCENE_H__
#define __INDOOR_LEVEL_SCENE_H__

#include <vector>
#include "IScene.h"

class GameObject;

class IndoorLevelScene: public IScene
{
public:
	IndoorLevelScene(const std::string& name);
	virtual ~IndoorLevelScene();

	virtual int  OnSceneLoad(ResourceManager* resManager) override;
	virtual void OnSceneExit() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

private:
	std::vector<GameObject*>	m_GameObjects;
	float m_TimeNow{ 0 };
};

#endif