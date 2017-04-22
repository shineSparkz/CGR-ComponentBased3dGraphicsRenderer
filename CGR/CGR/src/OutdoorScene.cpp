#include "OutdoorScene.h"

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

#include "CgrEngine.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"


OutDoorScene::OutDoorScene(const std::string& name) :
	IScene(name),
	m_GameObjects(),
	m_GoblinTransform(nullptr),
	m_TreeBillboardList(nullptr),
	m_TerrainConstructor(nullptr)
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
		static_cast<float>(Screen::ScreenWidth() / Screen::ScreenHeight()),									// Aspect
		0.1f,																								// Near
		500.0f																								// Far
	);

	m_CamTransform = cam->GetComponent<Transform>();

	m_Camera->AddSkybox(30.0f, TEX_SKYBOX_DEFAULT);

	// Set Pointer in renderer
	m_Renderer->SetSceneData(m_Camera, Vec3(0.1f));
	m_GameObjects.push_back(cam);

	// Create Animated Goblin
	bool createGob = !false;
	if (createGob)
	{
		// Create Animated goblin
		GameObject* goblin = new GameObject();
		m_GameObjects.push_back(goblin);
		m_GoblinTransform = goblin->AddComponent<Transform>();
		m_GoblinTransform->SetScale(Vec3(0.35f));
		m_GoblinTransform->RotateX(30.0f);
		m_GoblinTransform->SetPosition(Vec3(50, 30, -50));
		MeshRenderer* gmr = goblin->AddComponent<MeshRenderer>();
		gmr->SetMeshData(
			ANIM_MESH_GOBLIN,
			SHADER_ANIM,
			MATERIALS_GOBLIN,
			false,
			false,
			false,
			true);
		m_GoblinAnim = goblin->AddComponent<Animator>();
		m_GoblinAnim->StartAnimation(STAND);

		// Agh
		m_GoblinTransform->Update();

		// Calc Gob height once
		Vec3 gobMinVert = resManager->GetAnimMesh(ANIM_MESH_GOBLIN)->GetMinVertex();
		Vec3 gobMaxVert = resManager->GetAnimMesh(ANIM_MESH_GOBLIN)->GetMaxVertex();
		Vec3 gobCentre = gobMinVert + gobMaxVert / 2.0f;

		float r = Maths::Distance(
			Maths::Vec4To3(m_GoblinTransform->GetModelXform() * Vec4(gobMinVert, 1.0f)),
			Maths::Vec4To3(m_GoblinTransform->GetModelXform() * Vec4(gobCentre, 1.0f)));
		m_GoblinHeight = r + 4.0f;
	}

	// Directional Light
	GameObject* dlight = new GameObject();
	DirectionalLightC* dl = dlight->AddComponent<DirectionalLightC>();
	dl->SetLight(m_Renderer, Vec3(0, -0.8f, 0), Vec3(0.3f));
	m_DirLightHandle = dl;
	m_GameObjects.push_back(dlight);

	// Create Terrain
	bool createTerrain = !false;
	if (createTerrain)
	{
		// Try and create terrain
		if (!resManager->CheckMeshExists(MESH_TERRAIN))
		{
			std::vector<Vertex> verts;
			std::vector<uint32> indices;

			if (!m_TerrainConstructor)
				m_TerrainConstructor = new TerrainConstructor();

			if (!m_TerrainConstructor->CreateTerrain(verts,
				indices,
				resManager->GetShader(SHADER_TERRAIN_DEF),
				400,			// Sz X
				45.f,			// Sz Y
				400,			// Sz Z
				199,			// Sub U
				199,			// Sub V
				64,				// Tex U
				64,				// Tex V,
				"../resources/textures/terrain/heightmap.tga"
			))	
			/*
				if (!m_TerrainConstructor->CreateBez(verts,
					indices,
					resManager->GetShader(SHADER_TERRAIN_DEF),
					"../resources/textures/terrain/heightmap.tga",
					180.f,
					400,
					400,
					128,
					128,
					96,
					96,
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
				m_TerrainConstructor->GenerateRandomPositions(verts, positions, 500);

				// Need material and textures for bill board creation, which again I am not too happy with
				if (!m_TreeBillboardList->InitWithPositions(SHADER_BILLBOARD_FWD, TEX_GRASS_BILLBOARD, 4.5f, positions))
				{
					return GE_FALSE;
				}
			}

			// Create random Samourai's
			std::vector<Vec3> samarai_positions;
			m_TerrainConstructor->GenerateRandomPositions(verts, samarai_positions, 8);
			for (int i = 0; i < samarai_positions.size(); ++i)
			{
				GameObject* sam = new GameObject();
				
				Transform* t = sam->AddComponent<Transform>();
				t->SetScale(Vec3(0.25f));
				t->SetPosition(samarai_positions[i] + Vec3(0,8,0));
				t->Rotate(Vec3(30, 0, 0));
				
				MeshRenderer* mr = sam->AddComponent<MeshRenderer>();
				mr->SetMeshData(
					ANIM_MESH_SAMOURAI,
					SHADER_ANIM,
					MATERIALS_SAMOURAI,
					false,
					false,
					false,
					true
				);

				Animator* a = sam->AddComponent<Animator>();
				a->StartAnimation(animType_t::WAVE);

				m_GameObjects.push_back(sam);
			}

			// Create crate boxes
			std::vector<Vec3> box_positions;
			m_TerrainConstructor->GenerateRandomPositions(verts, box_positions, 15);
			for (int i = 0; i < box_positions.size(); ++i)
			{
				GameObject* cube = new GameObject();
				Transform* ct = cube->AddComponent<Transform>();
				ct->SetPosition(box_positions[i] + Vec3(0, 1.5f, 0));
				ct->SetScale(Vec3(1.0f));
				ct->Rotate(Vec3(0));
				MeshRenderer* cmr = cube->AddComponent<MeshRenderer>();
				cmr->SetMeshData(
					MESH_ID_CUBE,
					SHADER_LIGHTING_FWD,
					MATERIALS_WOOD,
					false,
					false,
					false,
					false);

				m_GameObjects.push_back(cube);
			}
		}

		// Terrain game object
		GameObject* terrain = new GameObject();
		Transform* terT = terrain->AddComponent<Transform>();
		terT->SetPosition(Vec3(0.0f, 0, 0.0f));
		terT->SetScale(Vec3(1.0f));
		MeshRenderer* tmr = terrain->AddComponent<MeshRenderer>();
		tmr->SetMeshData(
			MESH_TERRAIN,
			SHADER_TERRAIN_DEF,
			MATERIALS_TERRAIN,
			false,
			true,
			true,
			false);
		m_GameObjects.push_back(terrain);
	}

	// Load Rocks mesh
	if (!resManager->CheckMeshExists(MESH_ROCKS))
	{
		if (!resManager->LoadMesh("rocks/rocks_01_model.obj", MESH_ROCKS, true, true, MATERIALS_ROCK))
		{
			WRITE_LOG("rock load failed", "error");
			return false;
		}
	}

	// Load Pistol Mesh
	if (!resManager->CheckMeshExists(MESH_PISTOL))
	{
		if (!resManager->LoadMesh("pistol/pistol_mesh.obj", MESH_PISTOL, true, false, MATERIALS_PISTOL))
		{
			WRITE_LOG("pistol load failed", "error");
			return false;
		}
	}

	// Rock game obj
	GameObject* rock = new GameObject();
	m_GameObjects.push_back(rock);
	Transform* rkt = rock->AddComponent<Transform>();
	rkt->SetPosition(Vec3(70.0f, 20, -120));
	rkt->SetScale(Vec3(0.2f));
	MeshRenderer* rmr = rock->AddComponent<MeshRenderer>();
	rmr->SetMeshData(MESH_ROCKS,
		SHADER_LIGHTING_FWD,
		MATERIALS_ROCK,
		true,
		false,
		false,
		false
	);

	// Pistol game obj
	GameObject* pistol = new GameObject();
	m_GameObjects.push_back(pistol);
	m_PistolTransform = pistol->AddComponent<Transform>();
	m_PistolTransform->SetPosition(Vec3(0.0f));
	m_PistolTransform->SetScale(Vec3(5.0f));
	MeshRenderer* pmr = pistol->AddComponent<MeshRenderer>();
	pmr->SetMeshData(MESH_PISTOL,
		SHADER_LIGHTING_FWD,
		MATERIALS_PISTOL,
		true,
		false,
		false,
		false
	);

	if (!resManager->CheckMaterialSetExists(MATERIALS_PISTOL))
	{
		std::map<unsigned, Material*> mats;

		mats[0] = new Material();

		mats[0]->diffuse_map = resManager->LoadAndGetTexture("../resources/meshes/pistol/pistol_diffuse.tga", GL_TEXTURE0);
		if (!mats[0]->diffuse_map)
		{
			WRITE_LOG("pistil diff map fail", "error");
			SAFE_DELETE(mats[0]);
			return false;
		}

		mats[0]->normal_map =  resManager->LoadAndGetTexture("../resources/meshes/pistol/pistol_normal.tga", GL_TEXTURE2);
		if (!mats[0]->normal_map)
		{
			WRITE_LOG("pistil norm map fail", "error");
			SAFE_DELETE(mats[0]);
			return false;
		}
		
		resManager->AddMaterialSet(MATERIALS_PISTOL, mats);
	}

	// Start Game objects
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
	SAFE_DELETE(m_TerrainConstructor);
}

void OutDoorScene::Update(float dt)
{
	const auto& d = m_DirLightHandle->GetDir();
	const float SPEED = 35.0f;
	bool move_request = false;

	// Control Goblin
	if (Input::Keys[GLFW_KEY_UP] == GLFW_PRESS || Input::Keys[GLFW_KEY_UP] == GLFW_REPEAT)
	{
		m_GoblinTransform->MovePosition(Vec3(0, 0, -SPEED * dt));
		move_request = true;
	}
	if (Input::Keys[GLFW_KEY_DOWN] == GLFW_PRESS || Input::Keys[GLFW_KEY_DOWN] == GLFW_REPEAT)
	{
		m_GoblinTransform->MovePosition(Vec3(0, 0, SPEED * dt));
		move_request = true;
	}
	if (Input::Keys[GLFW_KEY_LEFT] == GLFW_PRESS || Input::Keys[GLFW_KEY_LEFT] == GLFW_REPEAT)
	{
		m_GoblinTransform->MovePosition(Vec3(-SPEED * dt, 0, 0));
		move_request = true;
	}
	if (Input::Keys[GLFW_KEY_RIGHT] == GLFW_PRESS || Input::Keys[GLFW_KEY_RIGHT] == GLFW_REPEAT)
	{
		m_GoblinTransform->MovePosition(Vec3(SPEED * dt, 0, 0));
		move_request = true;
	}
	if (Input::Keys[GLFW_KEY_F] == GLFW_PRESS || Input::Keys[GLFW_KEY_F] == GLFW_REPEAT)
	{
		m_GoblinTransform->MovePosition(Vec3(0, SPEED * dt, 0));
	}
	if (Input::Keys[GLFW_KEY_V] == GLFW_PRESS || Input::Keys[GLFW_KEY_V] == GLFW_REPEAT)
	{
		m_GoblinTransform->MovePosition(Vec3(0, SPEED * dt, 0));
	}

	if (move_request && m_GoblinAnim->GetCurrentAnim() != RUN)
		m_GoblinAnim->StartAnimation(RUN);
	else if(!move_request && m_GoblinAnim->GetCurrentAnim() != STAND)
		m_GoblinAnim->StartAnimation(STAND);

	// Set Positions
	const Vec3& gob_pos = m_GoblinTransform->Position();
	m_GoblinTransform->SetPosition(Vec3(gob_pos.x, m_TerrainConstructor->GetHeightFromPosition(gob_pos) + m_GoblinHeight, gob_pos.z));
	
	// Toggle Poly mode
	if (Input::Keys[GLFW_KEY_P] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_Renderer->TogglePolygonMode();
		m_TimeNow = Time::ElapsedTime();
	}
	
	// Reload shaders
	if (Input::Keys[GLFW_KEY_F6] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		// Reload all of the shaders, the renderer will also set uniforms on default shaders
		m_Renderer->ReloadShaders();

		// Any custom shader uniform should be set here
		m_TerrainConstructor->OnReloadShaders();

		m_TimeNow = Time::ElapsedTime();
	}

	// Reduce billboard size
	if (Input::Keys[GLFW_KEY_T] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_TreeBillboardList->SetScale(m_TreeBillboardList->GetScale() + 1.0f);
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_G] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_TreeBillboardList->SetScale(m_TreeBillboardList->GetScale() - 1.0f);
		m_TimeNow = Time::ElapsedTime();
	}

	// Update Game objects
	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}

	// Set cam to terrain
	const Vec3& cam_pos = m_CamTransform->Position();
	m_CamTransform->SetPosition(Maths::LerpV3(m_CamTransform->Position(), Vec3(cam_pos.x, m_TerrainConstructor->GetHeightFromPosition(cam_pos) + m_GoblinHeight, cam_pos.z), dt * 5.0f));
	//m_CamTransform->SetPosition(Vec3(cam_pos.x, m_TerrainConstructor->GetHeightFromPosition(cam_pos) + m_GoblinHeight, cam_pos.z));
	m_CamTransform->Update();

	//m_PistolTransform->SetPosition( Maths::LerpV3(m_PistolTransform->Position(), m_Camera->Position() + (m_Camera->Forward() * 2.0f), dt * 5.0f));
	m_PistolTransform->SetPosition(m_Camera->Position() +
		m_Camera->Forward() +
		(m_Camera->Right() * 0.25f) +
		(-m_Camera->Up() * 0.45f));

	//m_PistolTransform->Rotate(Vec3(0,-m_Camera->Forward().z,0));
	const Vec3 e = m_CamTransform->Euler();
	const float offset = 0.1f;
	m_PistolTransform->Rotate(Vec3(-e.x - offset, e.y, e.z));

	m_PistolTransform->Update();
	//m_PistolTransform->Rotate(-m_Camera->Forward());
}

void OutDoorScene::Render()
{
	m_Renderer->Render(m_GameObjects, true);
	m_Renderer->RenderBillboardList(m_TreeBillboardList);
}

void OutDoorScene::RenderUI()
{
	//m_Renderer->RenderText(FONT_COURIER, "Angle: " + util::to_str(X), 8, 96);
	m_Renderer->RenderText(FONT_COURIER, "Toggle wire frame: [P]",   8, 64);
	m_Renderer->RenderText(FONT_COURIER, "Change billboards: [T/G]", 8, 96);
	m_Renderer->RenderText(FONT_COURIER, "Gob Pos: " + util::vec3_to_str(m_GoblinTransform->Position()), 8, 128);
}