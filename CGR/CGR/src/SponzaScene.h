#ifndef __SPONZA_SCENE_H__
#define __SPONZA_SCENE_H__

#include "IScene.h"
#include <vector>

class GameObject;
class MeshRenderer;
class DirectionalLightC;

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
	void createDragon(const Vec3& position);

private:
	std::vector<GameObject*>	m_GameObjects;
	float						m_TimeNow{ 0 };
	MeshRenderer*				m_SponzaMeshRen;
	DirectionalLightC*			m_DlightPtr;
	bool						m_UsingBumpmaps{ true };
};

#endif