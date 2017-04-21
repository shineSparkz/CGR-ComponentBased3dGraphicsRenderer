#include "SponzaScene.h"

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
#include "Input.h"

#include "CgrEngine.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"


SponzaScene::SponzaScene(const std::string& name) :
	IScene(name)
{
}

SponzaScene::~SponzaScene()
{
}

void SponzaScene::createGameObjects()
{
	// Create Male
	GameObject* male = new GameObject();
	Transform* maleT = male->AddComponent<Transform>();
	maleT->SetPosition(Vec3(-7.0f, 0.0f, -10.0f));
	maleT->SetScale(Vec3(14.0f));
	MeshRenderer* maleMr = male->AddComponent<MeshRenderer>();
	maleMr->SetMeshData(
		MESH_ID_MALE,
		SHADER_LIGHTING_FWD,
		MATERIALS_MALE,
		false,
		false,
		false,
		false);
	m_Handle = m_GameObjects.size();
	m_GameObjects.push_back(male);

	// Create Sponza
	GameObject* sponza = new GameObject();
	Transform* sponzat = sponza->AddComponent<Transform>();
	sponzat->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
	sponzat->SetScale(Vec3(0.2f));
	MeshRenderer* sponzaMr = sponza->AddComponent<MeshRenderer>();
	sponzaMr->SetMeshData(
		MESH_ID_SPONZA,
		SHADER_LIGHTING_FWD,
		MATERIALS_SPONZA,
		true,
		true,
		false,
		false);
	m_GameObjects.push_back(sponza);
	m_SponzaPtr = sponzaMr;

	// Point Lights
	GameObject* p1 = CgrEngine::CreatePointLight(m_Renderer, Vec3(20.0f, 15.0f, -20.0f), Vec3(0.7f,0,0), 0.02f);
	GameObject* p2 = CgrEngine::CreatePointLight(m_Renderer, Vec3(-50.0f, 15.0f, 30.0f), Vec3(0.7f), 0.02f);
	GameObject* p3 = CgrEngine::CreatePointLight(m_Renderer, Vec3(70.0f, 15.0f, 10.0f), Vec3(0.7f), 0.02f);

	if (p1)
		m_GameObjects.push_back(p1);
	if (p2)
		m_GameObjects.push_back(p2);
	if (p3)
		m_GameObjects.push_back(p3);

	/*
	GameObject* s1 = CgrEngine::CreateSpotLight(m_Renderer, m_Camera->Position(), Vec3(0, 1, 0), m_Camera->Forward(), 12.0f, 1);
	if (s1)
		m_GameObjects.push_back(s1);
	*/

	// Directional Light
	GameObject* dlight = new GameObject();
	DirectionalLightC* dl = dlight->AddComponent<DirectionalLightC>();
	dl->SetLight(m_Renderer, Vec3(0, -1, 0), Vec3(0.7f));
	m_GameObjects.push_back(dlight);
	
	/*
	GameObject* lava = new GameObject();
	Transform* lt = lava->AddComponent<Transform>();
	lt->SetPosition(Vec3(2, 5.0f, -3.0f));
	lt->SetScale(Vec3(3.0f, 3.0f, 3.0f));
	MeshRenderer* lmr = lava->AddComponent<MeshRenderer>();
	lmr->SetMesh(CUBE_MESH, m_Meshes[CUBE_MESH]->m_SubMeshes.size());
	lmr->AddTexture(LAVA_NOISE_TEX);
	//lmr->m_ShaderIndex = LAVA_SHADER;
	lmr->m_ShaderIndex = STD_DEF_GEOM_SHADER;
	m_GameObjects.push_back(lava);
	*/

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Start();
	}
}

int SponzaScene::OnSceneLoad(ResourceManager* resManager)
{
	// Load sponza
	if (!resManager->CheckMeshExists(MESH_ID_SPONZA))
	{
		if (!resManager->LoadMesh("sponza/sponza.obj", MESH_ID_SPONZA, true, true, MATERIALS_SPONZA))
			return GE_MAJOR_ERROR;
	}

	auto s = resManager->GetMesh(MESH_ID_SPONZA);

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
		static_cast<float>(Screen::ScreenWidth() / Screen::ScreenHeight()),			// Aspect
		0.1f,																								// Near
		500.0f																								// Far
	);

	m_Camera->AddSkybox(30.0f, TEX_SKYBOX_DEFAULT);
	// Set Pointer in renderer
	m_Renderer->SetSceneData(m_Camera, Vec3(0.1f));
	m_GameObjects.push_back(cam);

	// Add Other Objecst
	this->createGameObjects();

	return GE_OK;
}

void SponzaScene::OnSceneExit()
{
	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		SAFE_CLOSE(m_GameObjects[i]);
	}
	m_GameObjects.clear();
}

void SponzaScene::Update(float dt)
{
	auto HANDLE = m_Handle;
	float SPEED = 2;
	if (Input::Keys[GLFW_KEY_UP] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, 0, -SPEED));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_DOWN] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, 0, SPEED));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_LEFT] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(-SPEED, 0, 0));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_RIGHT] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(SPEED, 0, 0));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_F] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, SPEED, 0));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_V] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.1f)
	{
		m_GameObjects[HANDLE]->GetComponent<Transform>()->MovePosition(Vec3(0, -SPEED, 0));
		m_TimeNow = Time::ElapsedTime();
	}

	// Toggle Bump maps
	if (Input::Keys[GLFW_KEY_N] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_SponzaPtr->SetUseBumpMaps(!m_SponzaPtr->UsingBumpMaps());
		m_TimeNow = Time::ElapsedTime();
	}

	// Hack Test
	m_GameObjects.back()->GetComponent<DirectionalLightC>()->SetDirectionY(cosf(Time::ElapsedTime()));

	auto* spot = m_GameObjects[m_GameObjects.size() - 2]->GetComponent<SpotLightC>();
	if (spot)
	{
		spot->SetDirection(m_Camera->Forward());
		spot->SetPosition(m_Camera->Position());
	}

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void SponzaScene::Render()
{
	m_Renderer->Render(m_GameObjects, true);
}

void SponzaScene::RenderUI()
{
	m_Renderer->RenderText(FONT_COURIER, "Cam Pos: " + util::vec3_to_str(m_Camera->Position()), 8, 32);
	m_Renderer->RenderText(FONT_COURIER, "Cam Fwd: " + util::vec3_to_str(m_Camera->Forward()), 8, 64);
	m_Renderer->RenderText(FONT_COURIER, "Cam Up: " + util::vec3_to_str(m_Camera->Up()), 8, 96);
	m_Renderer->RenderText(FONT_COURIER, "[N] Toggle Bump maps: " + util::vec3_to_str(m_Camera->Up()), 8, 128);

}