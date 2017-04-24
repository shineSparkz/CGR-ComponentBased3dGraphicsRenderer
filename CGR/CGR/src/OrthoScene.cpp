#include "OrthoScene.h"

#include "Input.h"
#include "AnimMesh.h"
#include "Animator.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"
#include "ChaseCamera.h"
#include "GameObject.h"
#include "Screen.h"
#include "ResId.h"
#include "utils.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "ResourceManager.h"
#include "DirectionalLight.h"

OrthoScene::OrthoScene(const std::string& name) :
	IScene(name)
{
}

OrthoScene::~OrthoScene()
{
}

int OrthoScene::OnSceneLoad(ResourceManager* resManager)
{
	// Create floor
	for (int i = 0; i < 1; ++i)
	{
		GameObject* cube1 = new GameObject();
		Transform* cubet = cube1->AddComponent<Transform>();
		cubet->SetPosition(Vec3(i*64, -320.0f, -1.0f));
		cubet->SetScale(Vec3(2640.0f, 320, 1));
		MeshRenderer* cube1Mr = cube1->AddComponent<MeshRenderer>();
		cube1Mr->SetMeshData(
			MESH_ID_CUBE,
			SHADER_LIGHTING_FWD,
			MATERIALS_GRASS,
			false,
			false,
			false,
			false);
		m_GameObjects.push_back(cube1);
	}

	// Create Camera
	GameObject* cam = new GameObject();
	m_Camera = cam->AddComponent<ChaseCamera2D>();
	m_Camera->Start();
	m_Camera->Init(
		CamType::Orthographic,
		Vec3(-500.0f, -250.0f, 1.0f),																				// Pos
		Vec3(0.0f, 1.0f, 0.0f),																				// Up
		Vec3(1.0f, 0.0f, 0.0f),																				// Right
		Vec3(0.0f, 0.0f, -1.0f),																			// Fwd
		45.0f,																								// FOV
		static_cast<float>(Screen::ScreenWidth() / Screen::ScreenHeight()),			// Aspect
		0.1f,																								// Near
		100.0f																								// Far
	);
	
	// Set Pointer in renderer
	m_Renderer->SetSceneData(m_Camera, Vec3(0.1f));
	m_GameObjects.push_back(cam);


	// Create Animated goblin
	GameObject* goblin = new GameObject();
	m_GameObjects.push_back(goblin);
	m_GoblinTransform = goblin->AddComponent<Transform>();
	m_GoblinTransform->SetScale(Vec3(3.0f));
	m_GoblinTransform->RotateX(30.0f);
	m_GoblinTransform->SetPosition(Vec3(-450, 60, -50));
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

	// Follow goblin
	m_Camera->SetTarget(m_GoblinTransform);
	
	// Directional Light
	GameObject* dlight = new GameObject();
	DirectionalLightC* dl = dlight->AddComponent<DirectionalLightC>();
	dl->SetLight(m_Renderer, Vec3(0.1f, -0.8f, 0.1f), Vec3(0.5f), Vec3(400.0f, 400.0f, 300.0f));
	m_GameObjects.push_back(dlight);

	return GE_OK;
}

void OrthoScene::OnSceneExit()
{
	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		SAFE_CLOSE(m_GameObjects[i]);
	}
	m_GameObjects.clear();
}

void OrthoScene::Update(float dt)
{
	bool move_request = false;
	const float SPEED = 285.0f;
	const Vec3& gob_euler = m_GoblinTransform->Euler();

	if (Input::Keys[GLFW_KEY_LEFT] == GLFW_PRESS || Input::Keys[GLFW_KEY_LEFT] == GLFW_REPEAT)
	{
		m_GoblinTransform->MovePosition(Vec3(-SPEED * dt, 0, 0));
		m_GoblinTransform->Rotate(Vec3(gob_euler.x, -110.0f, gob_euler.z));
		move_request = true;
	}
	if (Input::Keys[GLFW_KEY_RIGHT] == GLFW_PRESS || Input::Keys[GLFW_KEY_RIGHT] == GLFW_REPEAT)
	{
		m_GoblinTransform->MovePosition(Vec3(SPEED * dt, 0, 0));
		m_GoblinTransform->Rotate(Vec3(gob_euler.x, 0.0f, gob_euler.z));
		move_request = true;
	}

	if (move_request && m_GoblinAnim->GetCurrentAnim() != RUN)
		m_GoblinAnim->StartAnimation(RUN);
	else if (!move_request && m_GoblinAnim->GetCurrentAnim() != STAND)
		m_GoblinAnim->StartAnimation(STAND);

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void OrthoScene::Render()
{
	m_Renderer->Render(m_GameObjects);	
}

void OrthoScene::RenderUI()
{
}