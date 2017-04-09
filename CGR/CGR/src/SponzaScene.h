#ifndef __SPONZA_SCENE_H__
#define __SPONZA_SCENE_H__

#include "IScene.h"
#include <vector>

class GameObject;

class SponzaScene : public IScene
{
public:
	SponzaScene(const std::string& name);
	virtual ~SponzaScene();

	virtual int  OnSceneLoad(ResourceManager* resManager) override;
	virtual void OnSceneExit() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

private:
	void createGameObjects();

private:
	std::vector<GameObject*> m_GameObjects;

};

#endif