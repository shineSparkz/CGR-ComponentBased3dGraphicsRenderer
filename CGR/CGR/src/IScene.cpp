#include "IScene.h"
#include "Renderer.h"

int IScene::OnSceneCreate(Renderer* renderer)
{
	m_Renderer = renderer;
	return GE_OK;
}