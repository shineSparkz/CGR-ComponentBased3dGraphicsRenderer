#include "SponzaScene.h"

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
#include "Input.h"

#include "CgrEngine.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

SponzaScene::SponzaScene(const std::string& name) :
	IScene(name),
	m_SponzaMeshRen(nullptr)
{
}

SponzaScene::~SponzaScene()
{
}

int SponzaScene::OnSceneLoad(ResourceManager* resManager)
{
	// ---- Load Resources ----
	if (!resManager->CheckMeshExists(MESH_ID_SPONZA))
	{
		if (!resManager->LoadMesh("sponza/sponza.obj", MESH_ID_SPONZA, true, true, MATERIALS_SPONZA))
			return GE_MAJOR_ERROR;
	}

	if (!resManager->CheckMeshExists(MESH_DRAGON))
	{
		if (!resManager->LoadMesh("dragon/dragon.obj", MESH_DRAGON, true, true, MATERIALS_DRAGON))
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
		800.0f																								// Far
	);

	// Set Pointer in renderer
	m_Camera->AddSkybox(30.0f, TEX_SKYBOX_DEFAULT);
	m_Renderer->SetSceneData(m_Camera, Vec3(0.1f));
	m_GameObjects.push_back(cam);


	// Create Male mesh
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
	m_GameObjects.push_back(male);

	// Create Sponza scene
	GameObject* sponza = new GameObject();
	Transform* sponzat = sponza->AddComponent<Transform>();
	sponzat->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
	sponzat->SetScale(Vec3(0.2f));
	m_SponzaMeshRen = sponza->AddComponent<MeshRenderer>();
	m_SponzaMeshRen->SetMeshData(
		MESH_ID_SPONZA,
		SHADER_LIGHTING_FWD,
		MATERIALS_SPONZA,
		true,
		true,
		false,
		false);
	m_GameObjects.push_back(sponza);

	// Create some dragons
	createDragon(Vec3(50, 2, 100));
	createDragon(Vec3(-50, 2, 0));
	createDragon(Vec3(25, 70, -300));
	createDragon(Vec3(150, 90, -100));
	createDragon(Vec3(-150, 30, 100));

	// Point Lights
	GameObject* p1 = CgrEngine::CreatePointLight(m_Renderer, Vec3(20.0f, 15.0f, -20.0f), Vec3(0.5f, 0, 0), 0.02f);
	GameObject* p2 = CgrEngine::CreatePointLight(m_Renderer, Vec3(-50.0f, 15.0f, 30.0f), Vec3(0.5f), 0.02f);
	GameObject* p3 = CgrEngine::CreatePointLight(m_Renderer, Vec3(70.0f, 15.0f, 10.0f), Vec3(0.5f), 0.02f);

	if (p1)
		m_GameObjects.push_back(p1);
	if (p2)
		m_GameObjects.push_back(p2);
	if (p3)
		m_GameObjects.push_back(p3);


	// Create Spot Light
	GameObject* s1 = CgrEngine::CreateSpotLight(m_Renderer, Vec3(0, 30, 0), Vec3(0, 0.1f, 0), Vec3(0.1f, -0.9f, 0.1f), 0.2f, 1);
	if (s1)
		m_GameObjects.push_back(s1);

	// Directional Light
	GameObject* dlight = new GameObject();
	DirectionalLightC* dl = dlight->AddComponent<DirectionalLightC>();
	dl->SetLight(m_Renderer, Vec3(0.1f, -0.8f, 0.1f), Vec3(0.2f), Vec3(300.0f, 300.0f, 200.0f));
	m_GameObjects.push_back(dlight);

	// Start Game objects
	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Start();
	}

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
	// Toggle Shadows
	if (Input::Keys[GLFW_KEY_1] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_SponzaMeshRen->SetReceiveShadows(!m_SponzaMeshRen->ReceivingShadows());
		m_TimeNow = Time::ElapsedTime();
	}

	// Toggle Bump maps
	if (Input::Keys[GLFW_KEY_2] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_SponzaMeshRen->SetUseBumpMaps(!m_SponzaMeshRen->UsingBumpMaps());
		m_TimeNow = Time::ElapsedTime();
	}

	// Toggle Wire frame
	if (Input::Keys[GLFW_KEY_3] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_Renderer->TogglePolygonMode();
		m_TimeNow = Time::ElapsedTime();
	}

	// Toggle Deferred
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

	auto* spot = m_GameObjects[m_GameObjects.size() - 2]->GetComponent<SpotLightC>();
	
	// Toggle spot light
	if (Input::Keys[GLFW_KEY_P] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		spot->ToggleLight();
		m_TimeNow = Time::ElapsedTime();
	}

	// Move spot light
	if (spot)
	{
		spot->SetDirection((Vec3(cosf(Time::ElapsedTime() * 0.1f), -1.0f, sinf(Time::ElapsedTime() * 0.1f))));
	}

	// Update Objects and components
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
	m_Renderer->RenderText(FONT_COURIER, "[1] Toggle shadows ", 8, 64);
	m_Renderer->RenderText(FONT_COURIER, "[2] Toggle Bump maps ", 8, 96);
	m_Renderer->RenderText(FONT_COURIER, "[3] Toggle wire frame mode ", 8, 128);
	m_Renderer->RenderText(FONT_COURIER, "[4] Toggle deferred ", 8, 160);
	m_Renderer->RenderText(FONT_COURIER, "[5] Toggle normal displays ", 8, 192);
	m_Renderer->RenderText(FONT_COURIER, "[P] Toggle spot light ", 8, 224);
}

void SponzaScene::createDragon(const Vec3& position)
{
	GameObject* dragon = new GameObject();
	Transform* t = dragon->AddComponent<Transform>();
	t->SetScale(Vec3(1.0f));
	t->SetPosition(position);
	MeshRenderer* mr = dragon->AddComponent<MeshRenderer>();
	mr->SetMeshData(
		MESH_DRAGON,
		SHADER_LIGHTING_FWD,
		MATERIALS_DRAGON,
		false,
		false,
		false,
		false);
	m_GameObjects.push_back(dragon);
}