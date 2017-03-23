#include "OutdoorScene.h"

OutDoorScene::OutDoorScene(const std::string& name) :
	IScene(name)
{
}

OutDoorScene::~OutDoorScene()
{
}

int OutDoorScene::OnSceneCreate()
{
	return GE_OK;
}

int OutDoorScene::OnSceneLoad()
{
	return GE_OK;
}

void OutDoorScene::OnSceneExit()
{
}

void OutDoorScene::Update(float dt)
{
}

void OutDoorScene::Render(Renderer* renderer)
{
}