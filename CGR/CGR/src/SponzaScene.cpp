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

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

GameObject* createPointLight(const Vec3& position, const Vec3& colour, float intensity)
{
	const float ATTEN_CONST = 0.3f;
	const float ATTEN_LIN = 0.0174f;
	const float ATTEN_QUAD = 0.000080f;

	GameObject* pointLight = new GameObject();
	PointLightC* light = pointLight->AddComponent<PointLightC>();

	if (!light->SetLight(position, colour, intensity, ATTEN_CONST, ATTEN_LIN, ATTEN_QUAD))
	{
		SAFE_CLOSE(pointLight);
		return nullptr;
	}

	return pointLight;
}

GameObject* createSpotLight(const Vec3& position, const Vec3& colour, const Vec3& direction, float angle, int switchedOn)
{
	const float ATTEN_CONST = 0.3f;
	const float ATTEN_LIN = 0.0174f;
	const float ATTEN_QUAD = 0.000080f;

	GameObject* spotLight = new GameObject();
	SpotLightC* light = spotLight->AddComponent<SpotLightC>();

	if (!light->SetLight(position, direction, colour, angle, ATTEN_CONST, ATTEN_LIN, ATTEN_QUAD, switchedOn))
	{
		SAFE_CLOSE(spotLight);
		return nullptr;
	}

	return spotLight;
}



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
	cubet->SetPosition(Vec3(-7.0f, 0.0f, -10.0f));
	cubet->SetScale(Vec3(14.0f));
	MeshRenderer* cube1Mr = cube1->AddComponent<MeshRenderer>();
	cube1Mr->SetMesh(MALE_MESH, Renderer::Instance()->GetNumSubMeshesInMesh(MALE_MESH));
	cube1Mr->AddTexture(MALE_TEX1,1);
	cube1Mr->AddTexture(MALE_TEX2,0);
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

	GameObject* p1 = createPointLight(Vec3(20.0f, 3.0f, -20.0f), Vec3(0.7f), 0.2f);
	GameObject* p2 = createPointLight(Vec3(-50.0f, 3.0f, 30.0f), Vec3(0.7f), 0.2f);
	GameObject* p3 = createPointLight(Vec3(70.0f, 3.0f, 10.0f), Vec3(0.7f), 0.2f);

	if (p1)
		m_GameObjects.push_back(p1);
	if (p2)
		m_GameObjects.push_back(p2);
	if (p3)
		m_GameObjects.push_back(p3);

	GameObject* s1 = createSpotLight(m_Camera->Position(), Vec3(0, 1, 0), m_Camera->Forward(), 30.0f, 1);
	if (s1)
		m_GameObjects.push_back(s1);

	GameObject* dlight = new GameObject();
	DirectionalLightC* dl = dlight->AddComponent<DirectionalLightC>();
	dl->SetLight(Vec3(0, -1, 0), Vec3(0.7f));
	m_GameObjects.push_back(dlight);
	
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

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Start();
	}
}

int SponzaScene::OnSceneLoad()
{
	// Load sponza ( New resource)
	if (!ResourceManager::Instance()->LoadMesh("sponza/sponza.obj", SPONZA_MESH, false, true))
		return GE_MAJOR_ERROR;

	// Create Camera
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
	Renderer::Instance()->SetSceneData(m_Camera, Vec3(0.1f));
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

void SponzaScene::Render(Renderer* renderer)
{
	renderer->Render(m_GameObjects);

	renderer->RenderText(FONT_COUR, "Cam Pos: " + util::vec3_to_str(m_Camera->Position()), 8, 16);
	renderer->RenderText(FONT_COUR, "Cam Fwd: " + util::vec3_to_str(m_Camera->Forward()), 8, 32);
	renderer->RenderText(FONT_COUR, "Cam Up: " + util::vec3_to_str(m_Camera->Up()), 8, 48);
	renderer->RenderText(FONT_COUR, "Num Verts: " + util::to_str(Mesh::NumVerts), 8, 96, FontAlign::Left, Colour::Green());
	renderer->RenderText(FONT_COUR, "Num Meshes: " + util::to_str(Mesh::NumMeshes), 8, 128, FontAlign::Left, Colour::Green());
}