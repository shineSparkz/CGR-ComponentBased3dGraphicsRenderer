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
		Vec3(30.0f, 70.0f,-31.0f),																			// Pos
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

	GameObject* cube = new GameObject();
	Transform* ct = cube->AddComponent<Transform>();
	ct->SetPosition(Vec3(20.0f, 60.f, -20.0f));
	ct->SetScale(Vec3(1.0f));
	MeshRenderer* cmr = cube->AddComponent<MeshRenderer>();
	cmr->SetMesh(MESH_ID_CUBE);
	cmr->SetMaterialSet(MATERIALS_BRICKS);
	cmr->SetToUseBumpMaps(false);
	m_GameObjects.push_back(cube);

	// Directional Light
	GameObject* dlight = new GameObject();
	DirectionalLightC* dl = dlight->AddComponent<DirectionalLightC>();
	dl->SetLight(m_Renderer, Vec3(0, -0.8f, 0), Vec3(0.7f));
	m_DirLightHandle = dl;
	m_GameObjects.push_back(dlight);

	//if (!m_Surface)
	{
		m_Surface[0] = new SurfaceMesh();
		m_Surface[1] = new SurfaceMesh();
	}

	// Per Pixel Surface
	m_Surface[0]->Create(
		resManager->GetShader(SHADER_TERRAIN_DEF), 
		MATERIALS_TERRAIN,
		200,			// Sz X
		50.f,			// Sz Y
		200,			// Sz Z
		199,			// Sub U
		199,			// Sub V
		16,				// Tex U
		16,				// Tex V
		"../resources/textures/terrain/heightmap.tga"
	);

	// Bezier Surface
	m_Surface[1]->CreateBez(
		resManager->GetShader(SHADER_TERRAIN_DEF),
		MATERIALS_TERRAIN,
		"../resources/textures/terrain/heightmap.tga",
		50.f,
		500,
		500,
		128,
		128,
		16,
		16,
		true
	);

	// Terrain
	std::vector<Vec3> billboardPositions;

	// Load Billboard list
	if (!m_TreeBillboardList)
	{
		m_TreeBillboardList = new BillboardList();

		// Need material and textures for bill board creation, which again I am not too happy with
		if (!m_TreeBillboardList->InitWithPositions(resManager->GetShader(SHADER_BILLBOARD_FWD), TEX_GRASS_BILLBOARD, 0.5f, billboardPositions))
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

	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_Surface); ++i)
	{
		SAFE_DELETE(m_Surface[i]);
	}
	
	SAFE_DELETE(m_TreeBillboardList);
}

void OutDoorScene::Update(float dt)
{
	const auto& d = m_DirLightHandle->GetDir();

	if (Input::Keys[GLFW_KEY_UP] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_DirLightHandle->SetDirectionY(Maths::Max(-1.0f, d.y - 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_DOWN] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_DirLightHandle->SetDirectionY(Maths::Min(1.0f, d.y + 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_LEFT] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_DirLightHandle->SetDirectionX(Maths::Max(-1.0f, d.x - 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_RIGHT] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_DirLightHandle->SetDirectionX(Maths::Min(1.0f, d.x + 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_F] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_DirLightHandle->SetDirectionZ(Maths::Max(-1.0f, d.z - 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_V] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_DirLightHandle->SetDirectionZ(Maths::Min(1.0f, d.z + 0.1f));
		m_TimeNow = Time::ElapsedTime();
	}
	
	// Set Wireframe mode
	if (Input::Keys[GLFW_KEY_P] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_WireFrame = !m_WireFrame;
		m_TimeNow = Time::ElapsedTime();
	}

	// Set Fill mode
	if (Input::Keys[GLFW_KEY_O] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		m_RenderSurface = !m_RenderSurface;
		m_TimeNow = Time::ElapsedTime();
	}


	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void OutDoorScene::Render()
{
	m_Renderer->Render(m_GameObjects);

	if (m_RenderSurface)
	{
		glPolygonMode(GL_FRONT_AND_BACK, m_WireFrame ? GL_FILL : GL_LINE);
		m_Renderer->RenderSurface(m_Surface[0], Vec3(0, 0, 0));
		m_Renderer->RenderSurface(m_Surface[1], Vec3(0, 0, -500));
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//m_Renderer->RenderBillboardList(m_TreeBillboardList);
}

void OutDoorScene::RenderUI()
{
	m_Renderer->RenderText(FONT_COURIER, "Point Light Pos: " + util::vec3_to_str(m_DirLightHandle->GetDir()), 8, 64);
	m_Renderer->RenderText(FONT_COURIER, "Wire frame mode [P]", 8, 96);
	m_Renderer->RenderText(FONT_COURIER, "Render SUrface [O]", 8, 128);
}