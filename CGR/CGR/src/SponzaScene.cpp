#include "SponzaScene.h"

#include "Renderer.h"
#include "Camera.h"
#include "GameObject.h"
#include "Screen.h"
#include "ResId.h"
#include "utils.h"
#include "Transform.h"
#include "MeshRenderer.h"

SponzaScene::SponzaScene(const std::string& name) :
	IScene(name)
{
}

SponzaScene::~SponzaScene()
{
}

int SponzaScene::OnSceneCreate()
{
	// Currently Unused
	return GE_OK;
}

void SponzaScene::createGameObjects()
{
	// This will be moved
	/*
	GameObject* male = new GameObject();
	Transform* maleT = male->AddComponent<Transform>();
	maleT->SetPosition(Vec3(-0.5f, -1.5f, -1.0f));
	maleT->SetScale(Vec3(1.0f));
	MeshRenderer* maleMr = male->AddComponent<MeshRenderer>();
	maleMr->SetMesh(MALE_MESH, m_Meshes[MALE_MESH]->m_SubMeshes.size());
	maleMr->AddTexture(MALE_TEX2, 0);
	maleMr->AddTexture(FAKE_NORMAL_TEX, 0);
	maleMr->AddTexture(MALE_TEX1, 1);
	maleMr->AddTexture(FAKE_NORMAL_TEX, 1);
	maleMr->m_ShaderIndex = STD_DEF_GEOM_SHADER;
	m_GameObjects.push_back(male);
	*/

	/*
	GameObject* dino = new GameObject();
	Transform* dinoT = dino->AddComponent<Transform>();
	dinoT->SetPosition(Vec3(1.8f, -1.5f, -1.0f));
	dinoT->SetScale(Vec3(0.05f));
	MeshRenderer* dinoMr = dino->AddComponent<MeshRenderer>();
	dinoMr->SetMesh(DINO_MESH, m_Meshes[DINO_MESH]->m_SubMeshes.size());
	dinoMr->AddTexture(DINO_TEX);
	dinoMr->AddTexture(FAKE_NORMAL_TEX);
	dinoMr->m_ShaderIndex = STD_DEF_GEOM_SHADER;
	m_GameObjects.push_back(dino);
	*/

	GameObject* cube1 = new GameObject();
	Transform* cubet = cube1->AddComponent<Transform>();
	cubet->SetPosition(Vec3(0.0f, 1.0f, -10.0f));
	cubet->SetScale(Vec3(1.0f));
	MeshRenderer* cube1Mr = cube1->AddComponent<MeshRenderer>();
	cube1Mr->SetMesh(CUBE_MESH, Renderer::Instance()->GetNumSubMeshesInMesh(CUBE_MESH));
	cube1Mr->AddTexture(BRICK_TEX);
	cube1Mr->m_ShaderIndex = STD_DEF_GEOM_SHADER;
	m_GameObjects.push_back(cube1);

	GameObject* sponza = new GameObject();
	Transform* sponzat = sponza->AddComponent<Transform>();
	sponzat->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
	sponzat->SetScale(Vec3(0.2f));
	MeshRenderer* sponzaMr = sponza->AddComponent<MeshRenderer>();
	sponzaMr->SetMesh(SPONZA_MESH, Renderer::Instance()->GetNumSubMeshesInMesh(SPONZA_MESH));
	sponzaMr->m_ShaderIndex = STD_DEF_GEOM_SHADER;
	m_GameObjects.push_back(sponza);

	/*
	GameObject* cube2 = new GameObject();
	Transform* cube2t = cube2->AddComponent<Transform>();
	cube2t->SetPosition(Vec3(-2.0f, 0.0f, 0.0f));
	cube2t->SetScale(Vec3(1.0f));
	MeshRenderer* cube2Mr = cube2->AddComponent<MeshRenderer>();
	cube2Mr->SetMesh(CUBE_MESH, m_Meshes[CUBE_MESH]->m_SubMeshes.size());
	cube2Mr->AddTexture(BRICK_TEX);
	cube2Mr->AddTexture(FAKE_NORMAL_TEX);
	cube2Mr->m_ShaderIndex = STD_DEF_GEOM_SHADER;
	m_GameObjects.push_back(cube2);
	*/

	/*
	GameObject* floor = new GameObject();
	Transform* ft = floor->AddComponent<Transform>();
	ft->SetPosition(Vec3(0, -2.0f, -3.0f));
	ft->SetScale(Vec3(20.0f, 0.5f, 20.0f));
	//ft->RotateX(45);
	MeshRenderer* fmr = floor->AddComponent<MeshRenderer>();
	fmr->SetMesh(CUBE_MESH, m_Meshes[CUBE_MESH]->m_SubMeshes.size());
	fmr->AddTexture(WALL_TEX);
	//fmr->AddTexture(FAKE_NORMAL_TEX);
	fmr->m_ShaderIndex = STD_DEF_GEOM_SHADER;
	m_GameObjects.push_back(floor);
	*/

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
}

int SponzaScene::OnSceneLoad()
{
	GameObject* cam = new GameObject();
	m_Camera = cam->AddComponent<FlyCamera>();
	m_Camera->Start();
	m_Camera->Init(
		CamType::Perspective,
		Vec3(0.0f, 20.0f, 1.0f),																				// Pos
		Vec3(0.0f, 1.0f, 0.0f),																				// Up
		Vec3(1.0f, 0.0f, 0.0f),																				// Right
		Vec3(0.0f, 0.0f, -1.0f),																			// Fwd
		45.0f,																								// FOV
		static_cast<float>(Screen::Instance()->ScreenWidth() / Screen::Instance()->ScreenHeight()),			// Aspect
		0.1f,																								// Near
		500.0f																								// Far
	);

	m_Camera->AddSkybox(30.0f, SKYBOX_TEX);

	// Set Pointer in renderer
	Renderer::Instance()->SetCamera(m_Camera);

	m_GameObjects.push_back(cam);

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
	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void SponzaScene::Render(Renderer* renderer)
{
	renderer->Render(m_GameObjects);

	renderer->RenderText("Cam Pos: " + util::vec3_to_str(m_Camera->Position()), 8, 16);
	renderer->RenderText("Cam Fwd: " + util::vec3_to_str(m_Camera->Forward()), 8, 32);
	renderer->RenderText("Cam Up: " + util::vec3_to_str(m_Camera->Up()), 8, 48);
}