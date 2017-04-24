#include "SpaceScene.h"

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


SpaceScene::SpaceScene(const std::string& name) :
	IScene(name),
	m_GameObjects()
{
}

SpaceScene::~SpaceScene()
{
}

int SpaceScene::OnSceneLoad(ResourceManager* resManager)
{
	// Create new space skybox
	if (!resManager->CheckTextureExists(TEX_SKYBOX_SPACE))
	{
		std::string s[6] =
		{
			"../resources/textures/skybox/space/Right_MauveSpaceBox.tga",
			"../resources/textures/skybox/space/Left_MauveSpaceBox.tga",
			"../resources/textures/skybox/space/Down_MauveSpaceBox.tga",
			"../resources/textures/skybox/space/Up_MauveSpaceBox.tga",
			"../resources/textures/skybox/space/Back_MauveSpaceBox.tga",
			"../resources/textures/skybox/space/Front_MauveSpaceBox.tga"
		};

		if (!resManager->LoadCubeMap(s, TEX_SKYBOX_SPACE, GL_TEXTURE0))
		{
			WRITE_LOG("space cube map failed", "error");
			return GE_FALSE;
		}
	}

	// Create Camera
	GameObject* cam = new GameObject();
	m_GameObjects.push_back(cam);
	m_Camera = cam->AddComponent<FlyCamera>();
	m_Camera->Start();
	m_Camera->Init(
		CamType::Perspective,
		Vec3(30.0f, 70.0f, -31.0f),																			// Pos
		Vec3(0.0f, 1.0f, 0.0f),																				// Up
		Vec3(1.0f, 0.0f, 0.0f),																				// Right
		Vec3(0.0f, 0.0f, -1.0f),																			// Fwd
		45.0f,																								// FOV
		static_cast<float>(Screen::ScreenWidth() / Screen::ScreenHeight()),									// Aspect
		0.1f,																								// Near
		1000.0f																								// Far
	);

	m_CamTransform = cam->GetComponent<Transform>();
	m_Camera->AddSkybox(30.0f, TEX_SKYBOX_SPACE);

	// Set Pointer in renderer
	m_Renderer->SetSceneData(m_Camera, Vec3(0.1f));

	// Directional Light
	GameObject* dlight = new GameObject();
	DirectionalLightC* dl = dlight->AddComponent<DirectionalLightC>();
	dl->SetLight(m_Renderer, Vec3(0.1f, -0.8f, 0.1f), Vec3(0.5f), Vec3(400.0f, 400.0f, 300.0f));
	m_DirLightHandle = dl;
	m_GameObjects.push_back(dlight);

	// Load Spaceship mesh
	if (!resManager->CheckMeshExists(MESH_SPACESHIP))
	{
		if (!resManager->LoadMesh("spaceship/StarFighter.obj", MESH_SPACESHIP, true, true, MATERIALS_SPACESHIP))
		{
			WRITE_LOG("spaceship load failed", "error");
			return false;
		}
	}

	// Load Asteroids meshes
	if (!resManager->CheckMeshExists(MESH_ASTEROID_1))
	{
		if (!resManager->LoadMesh("asteroid/Asteroid_01_S/Asteroid_01_S.obj", MESH_ASTEROID_1, true, true, MATERIALS_ASTEROID_1))
		{
			WRITE_LOG("asteroid 1 load failed", "error");
			return false;
		}
	}

	if (!resManager->CheckMeshExists(MESH_ASTEROID_2))
	{
		if (!resManager->LoadMesh("asteroid/Asteroid_02_M/Asteroid_02_M.obj", MESH_ASTEROID_2, true, true, MATERIALS_ASTEROID_2))
		{
			WRITE_LOG("spaceship load failed", "error");
			return false;
		}
	}

	// Create Spaceship object
	GameObject* ss = new GameObject();
	m_GameObjects.push_back(ss);
	Transform* sst = ss->AddComponent<Transform>();
	sst->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
	sst->SetScale(Vec3(0.1f));
	MeshRenderer* smr = ss->AddComponent<MeshRenderer>();
	smr->SetMeshData(MESH_SPACESHIP,
		SHADER_LIGHTING_FWD,
		MATERIALS_SPACESHIP,
		true,
		false,
		false,
		false
	);
	ShipController* ship_cont = ss->AddComponent<ShipController>();
	ship_cont->SetCam(m_CamTransform);

	for (int i = 0; i < 30; ++i)
	{
		GameObject* a1 = new GameObject();
		m_GameObjects.push_back(a1);
		Transform* at = a1->AddComponent<Transform>();

		int ran = random::RandomRange(0, 2);
		size_t mesh = ran > 0 ? MESH_ASTEROID_1 : MESH_ASTEROID_2;
		size_t mat = ran > 0 ? MATERIALS_ASTEROID_1 : MATERIALS_ASTEROID_2;

		at->SetPosition(Vec3((float)random::RandomRange(0,500) - 250.0f, (float)random::RandomRange(0, 500) - 250.0f, -(float)random::RandomRange(0, 500) + 250.0f));
		at->SetScale(Vec3(0.9f));
		
		MeshRenderer* amr = a1->AddComponent<MeshRenderer>();
		amr->SetMeshData(mesh,
			SHADER_LIGHTING_FWD,
			mat,
			true,
			true,
			false,
			false
		);
	}

	// Start Game objects
	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Start();
	}

	return GE_OK;
}

void SpaceScene::OnSceneExit()
{
	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		SAFE_CLOSE(m_GameObjects[i]);
	}
	m_GameObjects.clear();
}

void SpaceScene::Update(float dt)
{
	const auto& d = m_DirLightHandle->GetDir();
	const float SPEED = 35.0f;
	bool move_request = false;

	const float ASTEROID_SPEED = 0.1f;
	for (int i = 4; i < m_GameObjects.size(); ++i)
	{
		Transform* t = m_GameObjects[i]->GetComponent<Transform>();
		if (t)
		{
			t->RotateZ(1.0f * ASTEROID_SPEED * dt);
			t->RotateX(1.0f * ASTEROID_SPEED * dt);
		}
	}

	// Control dir light
	// Temp
	if (Input::Keys[GLFW_KEY_1] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		Vec3 lcd = m_DirLightHandle->GetDir();
		m_DirLightHandle->SetDirection(Vec3(Maths::Max(-1.0f, lcd.x - 0.1f), lcd.y, lcd.z));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_2] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		Vec3 lcd = m_DirLightHandle->GetDir();
		m_DirLightHandle->SetDirection(Vec3(Maths::Min(1.0f, lcd.x + 0.1f), lcd.y, lcd.z));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_3] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		Vec3 lcd = m_DirLightHandle->GetDir();
		m_DirLightHandle->SetDirection(Vec3(lcd.x, lcd.y, Maths::Max(-1.0f, lcd.z - 0.1f)));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_4] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		Vec3 lcd = m_DirLightHandle->GetDir();
		m_DirLightHandle->SetDirection(Vec3(lcd.x, lcd.y, Maths::Min(1.0f, lcd.z + 0.1f)));
		m_TimeNow = Time::ElapsedTime();
	}

	// Update Game objects
	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void SpaceScene::Render()
{
	m_Renderer->Render(m_GameObjects, true);
}

void SpaceScene::RenderUI()
{
	m_Renderer->RenderText(FONT_CONSOLA, "Used W,A,S,D,Q,E to control ship", 8, 64);
}