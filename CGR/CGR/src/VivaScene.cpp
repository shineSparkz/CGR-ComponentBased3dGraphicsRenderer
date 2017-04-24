#include "VivaScene.h"

#include <glm/gtx/euler_angles.hpp>

#include "Input.h"
#include "Renderer.h"
#include "Material.h"
#include "Mesh.h"
#include "Camera.h"
#include "FlyCamera.h"
#include "GameObject.h"
#include "Screen.h"
#include "ResId.h"
#include "utils.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Animator.h"
#include "ResourceManager.h"
#include "BillboardList.h"
#include "Terrain.h"
#include "AnimMesh.h"
#include "ShipController.h"

#include "CgrEngine.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"


VivaScene::VivaScene(const std::string& name) :
	IScene(name),
	m_GameObjects()
{
}

VivaScene::~VivaScene()
{
}

int VivaScene::OnSceneLoad(ResourceManager* resManager)
{
	// TODO : Set up camera here, add it to game object vector, can add skybox to it, and call SetSccene data on the renderer class

	// TODO : Create any lights here, a directional light is needed for shadows to work, max 10 spots, max 10 points, max one dir

	// TODO : Load any resources that you want, such as shaders, textures, materials, meshes, or animated meshes, fonts

	// TODO : Create game objects, attach components

	// TODO : Create terrain and/or bill boards here if desired

	// Start Game objects
	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Start();
	}

	return GE_OK;
}

void VivaScene::OnSceneExit()
{
	// TODO : Clean up any resources here

	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		SAFE_CLOSE(m_GameObjects[i]);
	}
	m_GameObjects.clear();
}

void VivaScene::Update(float dt)
{
	// TODO : Add any specific update stuff here

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void VivaScene::Render()
{
	bool withShadows = false;
	m_Renderer->Render(m_GameObjects, withShadows);
}

void VivaScene::RenderUI()
{
	// TODO : Add any strings you want rendering here
	m_Renderer->RenderText(FONT_CONSOLA, "Example Text", 8, 96);
}

