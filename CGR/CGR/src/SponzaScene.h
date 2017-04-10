#ifndef __SPONZA_SCENE_H__
#define __SPONZA_SCENE_H__

#include "IScene.h"
#include <vector>

class GameObject;
class MeshRenderer;

class SponzaScene : public IScene
{
public:
	SponzaScene(const std::string& name);
	virtual ~SponzaScene();

	int  OnSceneLoad(ResourceManager* resManager) override;
	void OnSceneExit() override;
	void Update(float dt) override;
	void Render() override;
	void RenderUI() override;

private:
	void createGameObjects();

private:
	std::vector<GameObject*>	m_GameObjects;
	MeshRenderer*				m_SponzaPtr;	// <-- Weak Ptr
	float						m_TimeNow{ 0 };
};

#endif