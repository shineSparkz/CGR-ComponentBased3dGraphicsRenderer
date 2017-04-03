#ifndef __OUTDOOR_SCENE_H__
#define __OUTDOOR_SCENE_H__

#include "IScene.h"
#include <vector>

class GameObject;

class OrthoScene : public IScene
{
public:
	OrthoScene(const std::string& name);
	virtual ~OrthoScene();

	virtual int  OnSceneCreate() override;
	virtual int  OnSceneLoad() override;
	virtual void OnSceneExit() override;
	virtual void Update(float dt) override;
	virtual void Render(Renderer* renderer) override;

private:
	std::vector<GameObject*> m_GameObjects;
};

#endif