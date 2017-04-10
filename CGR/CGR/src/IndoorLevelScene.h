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

	int  OnSceneLoad(ResourceManager* resManager) override;
	void OnSceneExit() override;
	void Update(float dt) override;
	void Render() override;
	void RenderUI() override;

private:
	std::vector<GameObject*>	m_GameObjects;
	float m_TimeNow{ 0 };
};

#endif