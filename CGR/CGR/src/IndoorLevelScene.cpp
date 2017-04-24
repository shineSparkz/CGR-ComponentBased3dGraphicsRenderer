#include "IndoorLevelScene.h"

#include "Input.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"
#include "FlyCamera.h"
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

IndoorLevelScene::IndoorLevelScene(const std::string& name) :
	IScene(name),
	m_GameObjects()
{
}

IndoorLevelScene::~IndoorLevelScene()
{
}

int IndoorLevelScene::OnSceneLoad(ResourceManager* resManager)
{
	// Load Mesh for level
	if (!resManager->CheckMeshExists(MESH_ID_LEVEL))
	{
		if (!resManager->LoadMesh("cs_assault/cs_assault.obj", MESH_ID_LEVEL, false, true, MATERIALS_LEVEL))
			return GE_MAJOR_ERROR;
	}

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
		static_cast<float>(Screen::ScreenWidth() / Screen::ScreenHeight()),									// Aspect
		0.1f,																								// Near
		500.0f																								// Far
	);

	m_Camera->AddSkybox(30.0f, TEX_SKYBOX_DEFAULT);
	
	// Set Pointer in renderer
	m_Renderer->SetSceneData(m_Camera, Vec3(0.1f));
	m_GameObjects.push_back(cam);


	// Create Meshes
	GameObject* level = new GameObject();
	Transform* levelt = level->AddComponent<Transform>();
	levelt->SetPosition(Vec3(0.0f, -180.0f, 0.0f));
	levelt->SetScale(Vec3(0.1f));
	m_LvlMeshRenderer = level->AddComponent<MeshRenderer>();
	m_LvlMeshRenderer->SetMeshData(
		MESH_ID_LEVEL,
		SHADER_LIGHTING_FWD,
		MATERIALS_LEVEL,
		false,
		true,
		false,
		false);
	m_GameObjects.push_back(level);

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
	dl->SetLight(m_Renderer, Vec3(0.1f, -0.8f, 0.1f), Vec3(0.7f), Vec3(150.0f, 150.0f, 150.0f));
	m_GameObjects.push_back(dlight);

	// Create Bumped cube at origin
	for(int i = 0; i < 20; ++i)
	{
		GameObject* cube = new GameObject();
		Transform* ct = cube->AddComponent<Transform>();

		float rx = random::RandFloat(0.0f, 80.0f);
		float ry = random::RandFloat(0.0f, 30.0f);
		float rz = random::RandFloat(0.0f, 80.0f);

		if (i < 19)
			ct->SetPosition(Vec3(rx, ry, -rz));
		else
			ct->SetPosition(Vec3(5.0f));

		ct->SetScale(Vec3(5.0f));
		MeshRenderer* cmr = cube->AddComponent<MeshRenderer>();
		cmr->SetMeshData(
			MESH_ID_CUBE,
			SHADER_LIGHTING_FWD,
			MATERIALS_BRICKS,
			true,
			false,
			false,
			false);
		m_Handle = m_GameObjects.size();
		m_GameObjects.push_back(cube);
	}

	return GE_OK;
}

void IndoorLevelScene::OnSceneExit()
{
	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		SAFE_CLOSE(m_GameObjects[i]);
	}

	m_GameObjects.clear();
}

void IndoorLevelScene::Update(float dt)
{
	auto HANDLE = m_Handle;
	float SPEED = 25.0f;
	if (Input::Keys[GLFW_KEY_UP] == GLFW_PRESS)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, 0, -SPEED * dt));
	}
	if (Input::Keys[GLFW_KEY_DOWN] == GLFW_PRESS)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, 0, SPEED * dt));
	}
	if (Input::Keys[GLFW_KEY_LEFT] == GLFW_PRESS)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(-SPEED * dt, 0, 0));
	}
	if (Input::Keys[GLFW_KEY_RIGHT] == GLFW_PRESS)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(SPEED * dt, 0, 0));
	}
	if (Input::Keys[GLFW_KEY_F] == GLFW_PRESS)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, SPEED * dt, 0));
	}
	if (Input::Keys[GLFW_KEY_V] == GLFW_PRESS)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, -SPEED * dt, 0));
	}

	// Toggle shadows
	if (Input::Keys[GLFW_KEY_1] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_LvlMeshRenderer->SetReceiveShadows(!m_LvlMeshRenderer->ReceivingShadows());
		m_TimeNow = Time::ElapsedTime();
	}

	// Toggle Bumps
	if (Input::Keys[GLFW_KEY_2] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		MeshRenderer* mr = m_GameObjects.back()->GetComponent<MeshRenderer>();
		if (mr)
		{
			mr->SetUseBumpMaps(!mr->UsingBumpMaps());
		}
		m_TimeNow = Time::ElapsedTime();
	}

	// Toggle Wireframe mode
	if (Input::Keys[GLFW_KEY_3] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_Renderer->TogglePolygonMode();
		m_TimeNow = Time::ElapsedTime();
	}

	// Change Mode (defered/forward)
	if (Input::Keys[GLFW_KEY_4] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_Renderer->ToggleShadingMode();
		m_TimeNow = Time::ElapsedTime();
	}

	// Display Normals
	if (Input::Keys[GLFW_KEY_5] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_Renderer->DisplayNormals(!m_Renderer->IsDisplayingNormals());
		m_TimeNow = Time::ElapsedTime();
	}

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void IndoorLevelScene::Render()
{
	m_Renderer->Render(m_GameObjects, true);
}

void IndoorLevelScene::RenderUI()
{
	m_Renderer->RenderText(FONT_COURIER, "[1] Toggle shadows ", 8, 64);
	m_Renderer->RenderText(FONT_COURIER, "[2] Toggle Bump maps ", 8, 96);
	m_Renderer->RenderText(FONT_COURIER, "[3] Toggle wire frame mode ", 8, 128);
	m_Renderer->RenderText(FONT_COURIER, "[4] Toggle deferred ", 8, 160);
	m_Renderer->RenderText(FONT_COURIER, "[5] Toggle normal displays ", 8, 192);
	m_Renderer->RenderText(FONT_COURIER, "[Arrows, F, V] Move Box", 8, 224);
}