#ifndef __ORTHO_SCENE_H__
#define __ORTHO_SCENE_H__

#include "IScene.h"
#include <vector>

class GameObject;

class OrthoScene : public IScene
{
public:
	OrthoScene(const std::string& name);
	virtual ~OrthoScene();

	virtual int  OnSceneLoad(ResourceManager* resManager) override;
	virtual void OnSceneExit() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

private:
	std::vector<GameObject*> m_GameObjects;
};

#endif