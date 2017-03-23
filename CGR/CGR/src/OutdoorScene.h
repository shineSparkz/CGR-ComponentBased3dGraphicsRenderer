#ifndef __OUTDOOR_SCENE_H__
#define __OUTDOOR_SCENE_H__

#include "IScene.h"

class OutDoorScene : public IScene
{
public:
	OutDoorScene(const std::string& name);
	virtual ~OutDoorScene();

	virtual int  OnSceneCreate() override;
	virtual int  OnSceneLoad() override;
	virtual void OnSceneExit() override;
	virtual void Update(float dt) override;
	virtual void Render(Renderer* renderer) override;

private:

};

#endif