#include "OrthoScene.h"

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

OrthoScene::OrthoScene(const std::string& name) :
	IScene(name)
{
}

OrthoScene::~OrthoScene()
{
}

int OrthoScene::OnSceneCreate()
{
	return GE_OK;
}

int OrthoScene::OnSceneLoad()
{

	// BG Quad
	bool withWuad = !false;
	if (withWuad)
	{
		GameObject* quad = new GameObject();
		Transform* quadT = quad->AddComponent<Transform>();
		quadT->SetPosition(Vec3(0.0f, 0.0f, -50.0f));
		quadT->SetScale(Vec3(850, 600, 1));
		MeshRenderer* quadMr = quad->AddComponent<MeshRenderer>();
		quadMr->SetMesh(MESH_ID_QUAD, Renderer::Instance()->GetNumSubMeshesInMesh(MESH_ID_QUAD));
		quadMr->AddTexture(TEX_GRASS);
		quadMr->SetShader(SHADER_LIGHTING_FWD);
		m_GameObjects.push_back(quad);
	}

	// Create floor
	for (int i = 0; i < 50; ++i)
	{
		GameObject* cube1 = new GameObject();
		Transform* cubet = cube1->AddComponent<Transform>();
		cubet->SetPosition(Vec3(i*64, 0.0f, -1.0f));
		cubet->SetScale(Vec3(32.0f, 32, 1));
		MeshRenderer* cube1Mr = cube1->AddComponent<MeshRenderer>();
		cube1Mr->SetMesh(MESH_ID_CUBE, Renderer::Instance()->GetNumSubMeshesInMesh(MESH_ID_CUBE));
		cube1Mr->AddTexture(TEX_GRASS);
		cube1Mr->SetShader(SHADER_LIGHTING_FWD);
		m_GameObjects.push_back(cube1);
	}

	// Create Camera
	GameObject* cam = new GameObject();
	m_Camera = cam->AddComponent<ChaseCamera2D>();
	m_Camera->Start();
	m_Camera->Init(
		CamType::Orthographic,
		Vec3(0.0f, 0.0f, 1.0f),																				// Pos
		Vec3(0.0f, 1.0f, 0.0f),																				// Up
		Vec3(1.0f, 0.0f, 0.0f),																				// Right
		Vec3(0.0f, 0.0f, -1.0f),																			// Fwd
		45.0f,																								// FOV
		static_cast<float>(Screen::Instance()->ScreenWidth() / Screen::Instance()->ScreenHeight()),			// Aspect
		0.1f,																								// Near
		100.0f																								// Far
	);

	//m_Camera->AddSkybox(30.0f, SKYBOX_TEX);
	
	// Set Pointer in renderer
	Renderer::Instance()->SetSceneData(m_Camera, Vec3(0.1f));
	m_GameObjects.push_back(cam);
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
	// Hack to make bg same pos as cam
	Vec3 camPos = m_Camera->Position();
	Vec3 bgScale = m_GameObjects[0]->GetComponent<Transform>()->Scale();
	m_GameObjects[0]->GetComponent<Transform>()->SetPosition(
		Vec3(camPos.x + (bgScale.x * 0.5f), camPos.y + (bgScale.y * 0.5f), -50));

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void OrthoScene::Render(Renderer* renderer)
{
	renderer->Render(m_GameObjects);

	renderer->RenderText(FONT_COURIER, "Cam Pos: " + util::vec3_to_str(m_Camera->Position()), 8, 16);
	renderer->RenderText(FONT_COURIER, "Cam Fwd: " + util::vec3_to_str(m_Camera->Forward()), 8, 32);
	renderer->RenderText(FONT_COURIER, "Cam Up: " + util::vec3_to_str(m_Camera->Up()), 8, 48);
	
}