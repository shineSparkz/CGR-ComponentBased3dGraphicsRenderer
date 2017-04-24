#ifndef __ORTHO_SCENE_H__
#define __ORTHO_SCENE_H__

#include "IScene.h"
#include <vector>

class GameObject;
class Transform;
class Animator;

class OrthoScene : public IScene
{
public:
	OrthoScene(const std::string& name);
	virtual ~OrthoScene();

	int  OnSceneLoad(ResourceManager* resManager) override;
	void OnSceneExit() override;
	void Update(float dt) override;
	void Render() override;
	void RenderUI() override;

private:
	std::vector<GameObject*>	m_GameObjects;
	Transform*					m_GoblinTransform{ nullptr };
	Animator*					m_GoblinAnim{ nullptr };
};

#endif