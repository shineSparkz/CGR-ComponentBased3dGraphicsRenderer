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
	m_TreeBillboardList(nullptr)
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
		Vec3(30.0f, 70.0f, -31.0f),																			// Pos
		Vec3(0.0f, 1.0f, 0.0f),																				// Up
		Vec3(1.0f, 0.0f, 0.0f),																				// Right
		Vec3(0.0f, 0.0f, -1.0f),																			// Fwd
		45.0f,																								// FOV
		static_cast<float>(Screen::ScreenWidth() / Screen::ScreenHeight()),			// Aspect
		0.1f,																								// Near
		500.0f																								// Far
	);

	m_Camera->AddSkybox(30.0f, TEX_SKYBOX_DEFAULT);

	// Set Pointer in renderer
	m_Renderer->SetSceneData(m_Camera, Vec3(0.1f));
	m_GameObjects.push_back(cam);

	// Create Cube
	for (int y = 0; y < 5; ++y)
	{
		for (int x = 0; x < 5; ++x)
		{
			GameObject* cube = new GameObject();
			Transform* ct = cube->AddComponent<Transform>();
			ct->SetPosition(Vec3(x * 5.0f, 10.f, -y * 5.0f));
			ct->SetScale(Vec3(1.0f));
			ct->Rotate(Vec3(0));
			MeshRenderer* cmr = cube->AddComponent<MeshRenderer>();
			cmr->SetMesh(MESH_ID_CUBE);
			cmr->SetMaterialSet(MATERIALS_BRICKS);
			cmr->SetToUseBumpMaps(false);
			cmr->SetShader(SHADER_LIGHTING_FWD);
			cmr->ReceiveShadows = false;
			m_GameObjects.push_back(cube);
		}
	}

	bool createFloor = !false;
	if(createFloor)
	{
		// Create Floor
		GameObject* cube = new GameObject();
		Transform* ct = cube->AddComponent<Transform>();
		ct->SetPosition(Vec3(10.0f, 0.f, 0.0f));
		//ct->SetPosition(Vec3(0.0f, 10.0f, 0.0f));
		ct->SetScale(Vec3(10.0f, 2.0f, 10.0f));
		MeshRenderer* cmr = cube->AddComponent<MeshRenderer>();
		cmr->SetMesh(MESH_ID_CUBE);
		cmr->SetMaterialSet(MATERIALS_BRICKS);
		cmr->SetToUseBumpMaps(false);
		cmr->SetShader(SHADER_LIGHTING_FWD);
		cmr->ReceiveShadows = true;
		m_GameObjects.push_back(cube);
	}


	// Directional Light
	GameObject* dlight = new GameObject();
	DirectionalLightC* dl = dlight->AddComponent<DirectionalLightC>();
	dl->SetLight(m_Renderer, Vec3(0, -0.8f, 0), Vec3(0.7f));
	m_DirLightHandle = dl;
	m_GameObjects.push_back(dlight);

	bool createTerrain = !false;

	if (createTerrain)
	{
		// Try and create terrain
		if (!resManager->CheckMeshExists(MESH_TERRAIN))
		{
			std::vector<Vertex> verts;
			std::vector<uint32> indices;

			TerrainConstructor tcs;

			if (!tcs.CreateTerrain(verts,
				indices,
				resManager->GetShader(SHADER_TERRAIN_DEF),
				200,			// Sz X
				25.f,			// Sz Y
				200,			// Sz Z
				199,			// Sub U
				199,			// Sub V
				16,				// Tex U
				16,				// Tex V,
				"../resources/textures/terrain/heightmap.tga"
			))
				
				/*
				if (!tcs.CreateBez(verts,
					indices,
					resManager->GetShader(SHADER_TERRAIN_DEF),
					"../resources/textures/terrain/heightmap.tga",
					80.f,
					400,
					400,
					128,
					128,
					64,
					64,
					!true
				))
					*/
			{
				WRITE_LOG("terrain creation failed", "error");
				return GE_FALSE;
			}

			if (!resManager->CreateMesh(MESH_TERRAIN, verts, indices, MATERIALS_TERRAIN))
			{
				WRITE_LOG("Failed to create terrain mesh resource", "error");
				return GE_FALSE;
			}

			// Load Billboard list
			if (!m_TreeBillboardList)
			{
				m_TreeBillboardList = new BillboardList();

				std::vector<Vec3> positions;
				tcs.GenerateRandomPositions(verts, positions, 500);

				// Need material and textures for bill board creation, which again I am not too happy with
				if (!m_TreeBillboardList->InitWithPositions(resManager->GetShader(SHADER_BILLBOARD_FWD), TEX_GRASS_BILLBOARD, 0.5f, positions))
				{
					return GE_FALSE;
				}

				//m_TreeBillboardList->Init(ResourceManager::Instance()->GetShader(SHADER_BILLBOARD_FWD), TEX_GRASS_BILLBOARD,
				//0.5f,	// scale
				//10,		// numX
				//10,		// numY
				//2.0f,	// spacing
				//14.0f,	// offset pos
				//-1.4f	// ypos
				//);
			}
		}

		// Terrain game object
		GameObject* terrain = new GameObject();
		Transform* terT = terrain->AddComponent<Transform>();
		terT->SetPosition(Vec3(0.0f, 0, 0.0f));
		//terT->Rotate(Vec3(90, 0, 0));
		terT->SetScale(Vec3(1.0f));
		MeshRenderer* tmr = terrain->AddComponent<MeshRenderer>();
		tmr->SetMesh(MESH_TERRAIN);//<-- TODO Use loaded terrain mesh
		tmr->MultiTextures = true;
		tmr->ReceiveShadows = true;
		tmr->SetMaterialSet(MATERIALS_TERRAIN);
		tmr->SetToUseBumpMaps(false);
		tmr->SetShader(SHADER_TERRAIN_DEF);
		m_GameObjects.push_back(terrain);
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
	
	SAFE_DELETE(m_TreeBillboardList);
}

void OutDoorScene::Update(float dt)
{
	const auto& d = m_DirLightHandle->GetDir();

	const size_t HANDLE = 1;

	if (Input::Keys[GLFW_KEY_UP] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, 0, -5));
		//m_DirLightHandle->SetDirectionY(Maths::Max(-1.0f, d.y - 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_DOWN] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, 0, 5));
		//m_DirLightHandle->SetDirectionY(Maths::Min(1.0f, d.y + 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_LEFT] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(-5, 0, 0));
		//m_DirLightHandle->SetDirectionX(Maths::Max(-1.0f, d.x - 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_RIGHT] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(5, 0, 0));
		//m_DirLightHandle->SetDirectionX(Maths::Min(1.0f, d.x + 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_F] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, 5, 0));
		//m_DirLightHandle->SetDirectionZ(Maths::Max(-1.0f, d.z - 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_V] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, -5, 0));

		//m_DirLightHandle->SetDirectionZ(Maths::Min(1.0f, d.z + 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}

	if (Input::Keys[GLFW_KEY_P] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_Renderer->TogglePolygonMode();
		m_TimeNow = Time::ElapsedTime();
	}

	if (Input::Keys[GLFW_KEY_F6] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_Renderer->ReloadShaders();
		m_TimeNow = Time::ElapsedTime();
	}

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void OutDoorScene::Render()
{
	m_Renderer->Render(m_GameObjects, true);
	m_Renderer->RenderBillboardList(m_TreeBillboardList);
}

void OutDoorScene::RenderUI()
{
	m_Renderer->RenderText(FONT_COURIER, "Toggle wire frame: [P]", 8, 64);
	//m_Renderer->RenderText(FONT_COURIER, "Point Light Pos: " + util::vec3_to_str(m_DirLightHandle->GetDir()), 8, 64);
}