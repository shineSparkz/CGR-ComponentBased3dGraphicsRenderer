#include "VivaScene.h"

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
#include "ShipController.h"
#include "EventManager.h"

#include "CgrEngine.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"


VivaScene::VivaScene(const std::string& name) :
	IScene(name),
	m_GameObjects()
{
}

VivaScene::~VivaScene()
{
}

int VivaScene::OnSceneLoad(ResourceManager* resManager)
{
	// TODO : Set up camera here, add it to game object vector, can add skybox to it, and call SetScene data on the renderer class
	GameObject* camera = new GameObject();
	m_GameObjects.push_back(camera);
	BaseCamera* fly = camera->AddComponent<FlyCamera>();
	fly->Init(CamType::Perspective, Vec3(0, 5, 3), Vec3(0, 1, 0), Vec3(1, 0, 0), Vec3(0, 0, 1), 45.0f, static_cast<float>(Screen::FrameBufferWidth() / Screen::FrameBufferHeight()), 0.1f, 600.0f);
	fly->AddSkybox(30.0f, TEX_SKYBOX_DEFAULT);
	m_Renderer->SetSceneData(fly, Vec3(0.01f));

	// TODO : Create any lights here, a directional light is needed for shadows to work, max 10 spots, max 10 points, max one dir
	GameObject* dirLightObj = new GameObject();
	m_GameObjects.push_back(dirLightObj);
	DirectionalLightC* dirLight = dirLightObj->AddComponent<DirectionalLightC>();
	dirLight->SetLight(m_Renderer, Vec3(0.0f, -0.9f, 0.1f), Vec3(0.5f), Vec3(100, 100, 100));

	//GameObject* point = CgrEngine::CreatePointLight(m_Renderer, Vec3(20, 55, -35), Vec3(0.8f, 0, 0), 0.1f);
	//if (point)
	//	m_GameObjects.push_back(point);

	// TODO : Load any resources that you want, such as shaders, materials, meshes, fonts

	// ---- Create a font ----
	if (!resManager->LoadFont("../resources/viva/olde_english.ttf", 100, 32))
	{
		WRITE_LOG("Failed to create font", "error");
		return GE_FATAL_ERROR;
	}

	// --- Create a shader ----
	Shader vertShader(GL_VERTEX_SHADER);
	Shader fragShader(GL_FRAGMENT_SHADER);
	vertShader.AddAttribute(ShaderAttrib{ 0, "vertex_position" });
	vertShader.AddAttribute(ShaderAttrib{ 1, "vertex_normal" });
	vertShader.AddAttribute(ShaderAttrib{ 2, "vertex_texcoord" });
	vertShader.AddAttribute(ShaderAttrib{ 3, "vertex_tangent" });
	
	if (!vertShader.LoadShader("../resources/viva/vert.glsl"))
	{
		WRITE_LOG("Failed to create vert shader", "error");
		return GE_FATAL_ERROR;
	}
	if (!fragShader.LoadShader("../resources/viva/frag.glsl"))
	{
		WRITE_LOG("Failed to create frag shader", "error");
		return GE_FATAL_ERROR;
	}
	std::vector<Shader> shaders;
	shaders.push_back(vertShader);
	shaders.push_back(fragShader);

	if (!resManager->CreateShaderProgram(shaders, 100))
	{
		WRITE_LOG("Failed to create shader program", "error");
		return GE_FATAL_ERROR;
	}

	// ---- Create Material ----
	std::map<unsigned, Material*> material;
	Texture* diffuse = resManager->LoadAndGetTexture("../resources/viva/treasure_chest.jpg", DIFFUSE_MAP_SAMPLER);
	Texture* normal  = resManager->LoadAndGetTexture("../resources/viva/treasure_chest_norm.jpg", NORMAL_MAP_SAMPLER);

	if (diffuse && normal)
	{
		material[0] = new Material();
		material[0]->diffuse_map = diffuse;
		material[0]->normal_map = normal;

		resManager->AddMaterialSet(100, material);
	}
	else
	{
		return GE_FATAL_ERROR;
	}
	
	// Create Mesh
	if (!resManager->LoadMesh("../viva/treasure_chest.obj", 100, true, false, 100))
	{
		WRITE_LOG("Failed to load mesh", "error");
		return GE_FATAL_ERROR;
	}
	
	// Create terrain and/or bill boards here if desired
	m_Terrain = new TerrainConstructor();
	std::vector<Vertex> verts;
	std::vector<unsigned> indices;
	if (!m_Terrain->CreateTerrain(verts, indices, resManager->GetShader(SHADER_TERRAIN_DEF), 50, 8, 90, 199, 199, 4, 4, "../resources/textures/terrain/heightmap.tga"))
	{
		WRITE_LOG("terrain construction failed", "error");
		return GE_FATAL_ERROR;
	}

	// Load mesh reosurce for it
	if (!resManager->CreateMesh(101, verts, indices, MATERIALS_TERRAIN))
	{
		WRITE_LOG("terrain mesh load fail", "error");
	}

	// Create a gameobject for terrain
	GameObject* terrain = new GameObject();
	m_GameObjects.push_back(terrain);
	terrain->AddComponent<Transform>();
	MeshRenderer* tmr = terrain->AddComponent<MeshRenderer>();
	tmr->SetMeshData(
		101,
		SHADER_TERRAIN_DEF,
		MATERIALS_TERRAIN,
		false,
		true,
		true,
		false);
	

	// Create game objects, attach components
	GameObject* obj = new GameObject();
	m_GameObjects.push_back(obj);
	Transform* t = obj->AddComponent<Transform>();
	t->SetPosition(Vec3(20.0f, 10.0f, -35.0f));
	t->SetScale(Vec3(0.5f));
	MeshRenderer* mr = obj->AddComponent<MeshRenderer>();
	mr->SetMeshData(
		100,						// Mesh Resource Index
		SHADER_LIGHTING_FWD,		// Shader Resource Index
		100,						// Material Resource Index
		true,						// Use Bump maps
		false,						// Receive shadows
		false,						// Has multiple diffuse textures
		false);						// Is Animated mesh

	// Start Game objects
	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Start();
	}

	m_Renderer->SetShadingMode(ShadingMode::Forward);

	return GE_OK;
}

void VivaScene::OnSceneExit()
{
	// TODO : Clean up any resources here

	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		SAFE_CLOSE(m_GameObjects[i]);
	}

	SAFE_DELETE(m_Terrain);

	m_GameObjects.clear();
}

void VivaScene::Update(float dt)
{
	// TODO : Add any specific update stuff here
	Transform* t = m_GameObjects.back()->GetComponent<Transform>();
	if (t)
	{
		if (Input::Keys[GLFW_KEY_UP] == GLFW_PRESS)
		{
			t->MovePosition(Vec3(0, 0, -10 * dt));
		}
		if (Input::Keys[GLFW_KEY_DOWN] == GLFW_PRESS)
		{
			t->MovePosition(Vec3(0, 0, 10 * dt));
		}
		if (Input::Keys[GLFW_KEY_LEFT] == GLFW_PRESS)
		{
			t->MovePosition(Vec3(-10 * dt, 0, 0));
		}
		if (Input::Keys[GLFW_KEY_RIGHT] == GLFW_PRESS)
		{
			t->MovePosition(Vec3(10 * dt, 0, 0));
		}
	}

	// Change shader
	if (Input::Keys[GLFW_KEY_P] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		MeshRenderer* mr = m_GameObjects.back()->GetComponent<MeshRenderer>();
		mr->SetShaderIndex(100);
		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_O] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		MeshRenderer* mr = m_GameObjects.back()->GetComponent<MeshRenderer>();
		mr->SetShaderIndex(SHADER_LIGHTING_FWD);
		m_TimeNow = Time::ElapsedTime();
	}

	// Change mesh
	if (Input::Keys[GLFW_KEY_I] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		MeshRenderer* mr = m_GameObjects.back()->GetComponent<MeshRenderer>();
		mr->SetMaterialIndex(100);
		mr->SetMeshIndex(100);
		mr->SetUseBumpMaps(true);

		m_GameObjects.back()->GetComponent<Transform>()->SetScale(Vec3(0.5f));

		m_TimeNow = Time::ElapsedTime();
	}
	if (Input::Keys[GLFW_KEY_U] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		MeshRenderer* mr = m_GameObjects.back()->GetComponent<MeshRenderer>();
		mr->SetMaterialIndex(MATERIALS_MALE);
		mr->SetMeshIndex(MESH_ID_MALE);
		mr->SetUseBumpMaps(false);

		m_GameObjects.back()->GetComponent<Transform>()->SetScale(Vec3(2.0f));

		m_TimeNow = Time::ElapsedTime();
	}

	if (Input::Keys[GLFW_KEY_F8] == GLFW_PRESS && Time::ElapsedTime() - m_TimeNow > 0.5f)
	{
		if (!m_Renderer->ReloadShaders())
		{
			EventManager::Instance()->SendEvent(EVENT_SHUTDOWN, nullptr);
			return;
		}
		
		m_TimeNow = Time::ElapsedTime();
	}

	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}
}

void VivaScene::Render()
{
	bool withShadows = true;
	m_Renderer->Render(m_GameObjects, withShadows);
}

void VivaScene::RenderUI()
{
	// TODO : Add any strings you want rendering here
	m_Renderer->RenderText(100, "This is our loaded font", 8, 96);
}

