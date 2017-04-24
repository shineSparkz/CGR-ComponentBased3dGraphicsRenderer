#ifndef __INDOOR_LEVEL_SCENE_H__
#define __INDOOR_LEVEL_SCENE_H__

#include <vector>
#include "IScene.h"

class GameObject;
class MeshRenderer;

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
	MeshRenderer*				m_LvlMeshRenderer;
	float						m_TimeNow{ 0 };
	size_t						m_Handle = 0;
};

#endif