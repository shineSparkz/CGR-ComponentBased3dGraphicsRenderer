#include "IndoorLevelScene.h"

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
		if (!resManager->LoadMesh("cs_assault/cs_assault.obj", MESH_ID_LEVEL, false, true))
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
		static_cast<float>(Screen::Instance()->ScreenWidth() / Screen::Instance()->ScreenHeight()),			// Aspect
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
	MeshRenderer* levelMr = level->AddComponent<MeshRenderer>();
	levelMr->SetMesh(MESH_ID_LEVEL, m_Renderer->GetNumSubMeshesInMesh(MESH_ID_LEVEL));
	levelMr->AddTexture(TEX_BRICKS);
	levelMr->SetShader(SHADER_GEOM_PASS_DEF);
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
	dl->SetLight(m_Renderer, Vec3(0, -1, 0), Vec3(0.7f));
	m_GameObjects.push_back(dlight);

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
	if (Input::Keys[GLFW_KEY_P] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_Renderer->ToggleShadingMode();
		m_TimeNow = Time::ElapsedTime();
	}

	m_GameObjects.back()->GetComponent<DirectionalLightC>()->SetDirectionY(cosf(Time::ElapsedTime() ));

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void IndoorLevelScene::Render()
{
	m_Renderer->Render(m_GameObjects);

	m_Renderer->RenderText(FONT_COURIER, "Cam Pos: " + util::vec3_to_str(m_Camera->Position()), 8, 16);
	m_Renderer->RenderText(FONT_COURIER, m_Renderer->GetShadingModeStr(), 8, 32);

}