#include "OutdoorScene.h"

#include "Input.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"
#include "GameObject.h"
#include "Screen.h"
#include "ResId.h"
#include "utils.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "ResourceManager.h"
#include "BillboardList.h"
#include "Terrain.h"

#include "CgrEngine.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

OutDoorScene::OutDoorScene(const std::string& name) :
	IScene(name),
	m_GameObjects(),
	m_TreeBillboardList(nullptr),
	m_Terrain(nullptr)
{
}

OutDoorScene::~OutDoorScene()
{
}

int OutDoorScene::OnSceneLoad(ResourceManager* resManager)
{
	// Create Camera
	GameObject* cam = new GameObject();
	m_Camera = cam->AddComponent<FlyCamera>();
	m_Camera->Start();
	m_Camera->Init(
		CamType::Perspective,
		Vec3(0.0f, 20.0f, 1.0f),																			// Pos
		Vec3(0.0f, 1.0f, 0.0f),																				// Up
		Vec3(1.0f, 0.0f, 0.0f),																				// Right
		Vec3(0.0f, 0.0f, -1.0f),																			// Fwd
		45.0f,																								// FOV
		static_cast<float>(Screen::Instance()->ScreenWidth() / Screen::Instance()->ScreenHeight()),			// Aspect
		0.1f,																								// Near
		500.0f																								// Far
	);

	m_Camera->AddSkybox(30.0f, TEX_SKYBOX_DEFAULT);
	// Set Pointer in renderer
	m_Renderer->SetSceneData(m_Camera, Vec3(0.1f));
	m_GameObjects.push_back(cam);

	// Point Lights
	GameObject* p1 = CgrEngine::CreatePointLight(m_Renderer, Vec3(20.0f, 15.0f, -20.0f), Vec3(0.7f, 0, 0), 0.02f);
	GameObject* p2 = CgrEngine::CreatePointLight(m_Renderer, Vec3(-50.0f, 15.0f, 30.0f), Vec3(0.7f), 0.02f);
	GameObject* p3 = CgrEngine::CreatePointLight(m_Renderer, Vec3(70.0f, 15.0f, 10.0f), Vec3(0.7f), 0.02f);

	if (p1)
		m_GameObjects.push_back(p1);
	if (p2)
		m_GameObjects.push_back(p2);
	if (p3)
		m_GameObjects.push_back(p3);

	// Directional Light
	GameObject* dlight = new GameObject();
	DirectionalLightC* dl = dlight->AddComponent<DirectionalLightC>();
	dl->SetLight(m_Renderer, Vec3(0, -1, 0), Vec3(0.7f));
	m_GameObjects.push_back(dlight);

	// Terrain
	std::vector<Vec3> billboardPositions;

	if (!m_Terrain)
		m_Terrain = new Terrain();

	unsigned int textures[5] = { TEX_TERRAIN1, TEX_TERRAIN2, TEX_TERRAIN3, TEX_TERRAIN4, TEX_TERRAIN5 };
	if (!m_Terrain->LoadFromHeightMapWithBillboards(
		"../resources/textures/terrain/heightmap.tga",
		resManager->GetShader(SHADER_TERRAIN_DEF),
		textures,
		Vec3(200, 30, 200),
		Vec3(1.0f),
		billboardPositions,
		50
	))
	{
		return GE_FALSE;
	}

	// Load Billboard list
	if (!m_TreeBillboardList)
	{
		m_TreeBillboardList = new BillboardList();

		// Need material and textures for bill board creation, which again I am not too happy with
		if (!m_TreeBillboardList->InitWithPositions(resManager->GetShader(SHADER_BILLBOARD_FWD), TEX_GRASS_BILLBOARD, 0.5f, billboardPositions))
		{
			return GE_FALSE;
		}

		/*
		m_TreeBillboardList->Init(ResourceManager::Instance()->GetShader(SHADER_BILLBOARD_FWD), TEX_GRASS_BILLBOARD,
		0.5f,	// scale
		10,		// numX
		10,		// numY
		2.0f,	// spacing
		14.0f,	// offset pos
		-1.4f	// ypos
		);
		*/
	}

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Start();
	}


	return GE_OK;
}

void OutDoorScene::OnSceneExit()
{
	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		SAFE_CLOSE(m_GameObjects[i]);
	}
	m_GameObjects.clear();

	SAFE_DELETE(m_Terrain);
	SAFE_DELETE(m_TreeBillboardList);
}

void OutDoorScene::Update(float dt)
{
	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void OutDoorScene::Render()
{
	m_Renderer->Render(m_GameObjects);
	m_Renderer->RenderTerrain(m_Terrain);
	m_Renderer->RenderBillboardList(m_TreeBillboardList);

	m_Renderer->RenderText(FONT_COURIER, m_Renderer->GetShadingModeStr(), 8, 32);
}