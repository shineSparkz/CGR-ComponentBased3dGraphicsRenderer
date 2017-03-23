#include "Renderer.h"

// STD
#include <sstream>
#include <ft2build.h>
#include FT_FREETYPE_H

// Other Graphics
#include "OpenGlLayer.h"
#include "Screen.h"
#include "Mesh.h"
#include "ShadowFrameBuffer.h"
#include "BillboardList.h"
#include "Terrain.h"
#include "Technique.h"
#include "Font.h"
#include "Texture.h"
#include "GBuffer.h"

// Utils
#include "LogFile.h"
#include "utils.h"
#include "math_utils.h"
#include "Image.h"
#include "TextFile.h"

// Game Object and Components
#include "GameObject.h"
#include "Transform.h"
#include "Camera.h"
#include "MeshRenderer.h"

#include "Input.h"
#include "ResId.h"

// Transforms -- for now
const Mat4 MALE_XFORM =  glm::translate(Mat4(1.0f), Vec3(-0.5f, -1.5f, -10.0f))  *  glm::scale(Mat4(1.0f), Vec3(1.0f));
const Mat4 DINO_XFORM =  glm::translate(Mat4(1.0f), Vec3(1.8f, -1.5f, -10.0f))   *  glm::scale(Mat4(1.0f), Vec3(0.05f));
const Mat4 CUBE1_XFORM = glm::translate(Mat4(1.0f), Vec3(0.0f, 1.0f, -10.0f))   *  glm::scale(Mat4(1.0f), Vec3(1.0f));
const Mat4 CUBE2_XFORM = glm::translate(Mat4(1.0f), Vec3(-2.0f, 0.0f, 0.0f))   *  glm::scale(Mat4(1.0f), Vec3(0.5f));
const Mat4 FLOOR_XFORM = glm::translate(Mat4(1.0f), Vec3(0.0f, -2.0f, -3.0f))  *  glm::scale(Mat4(1.0f), Vec3(30.0f, 0.5f, 30.0f));
const Mat4 LAVA_XFORM = glm::translate(Mat4(1.0f),  Vec3(0.0f, 4.0f, 0.0f)) *  glm::scale(Mat4(1.0f), Vec3(400.0f, 2.0f, 400.0f));


// ---- Globals ----
const Mat4 IDENTITY(1.0f);
const bool DO_SHADOWS = true;

const ShaderAttrib POS_ATTR{  0, "vertex_position" };
const ShaderAttrib NORM_ATTR{ 1, "vertex_normal" };
const ShaderAttrib TEX_ATTR{  2, "vertex_texcoord" };
const ShaderAttrib TAN_ATTR{  3, "vertex_tangent" };

// Temp Light stuff 
const Vec3 AMBIENT_LIGHT(0.1f);
float DIRECTION_ANGLE = 45.0f;
float ATTEN_CONST = 0.3f;
float ATTEN_LIN = 0.0174f;
float ATTEN_QUAD = 0.000080;

float CalcPointLightBSphere(const PointLight& Light)
{
	float MaxChannel = fmax(fmax(Light.intensity.x, Light.intensity.y), Light.intensity.z);

	float ret = (-Light.aLinear + sqrtf(Light.aLinear * Light.aLinear -
		4 * Light.aQuadratic* (Light.aQuadratic - 256 * MaxChannel * Light.ambient_intensity)))
		/
		(2 * Light.aQuadratic);
	return ret;
}

Renderer::Renderer() :
	m_Textures(),
	m_Font(nullptr),
	m_Gbuffer(nullptr),
	m_CameraPtr(nullptr),

	m_TreeBillboardList(nullptr),
	m_Terrain(nullptr)
{
}

bool Renderer::Init()
{	
	bool success = true;

	m_Query.Init(GL_TIME_ELAPSED);

	success &= setRenderStates();
	success &= setFrameBuffers();
	success &= setLights();
	success &= loadFonts();
	success &= loadTetxures();
	success &= loadMeshes();
	success &= createMaterials();

	// Create Terrain
	std::vector<Vec3> billboardPositions;
	m_Terrain = new Terrain();

	unsigned int textures[5] = { TERRAIN1_TEX, TERRAIN2_TEX, TERRAIN3_TEX, TERRAIN4_TEX, TERRAIN5_TEX };
	success &= m_Terrain->LoadFromHeightMapWithBillboards(
		"../resources/textures/terrain/heightmap.tga",
		m_Shaders[TERRAIN_SHADER],
		textures,
		Vec3(200, 30, 200),
		billboardPositions,
		500
	);

	// Need material and textures for bill board creation, which again I am not too happy with
	m_TreeBillboardList = new BillboardList();
	success &= m_TreeBillboardList->InitWithPositions(m_Shaders[BILLBOARD_SHADER], TREE_BILLBOARD_TEX, 0.5f, billboardPositions);

	m_TreeBillboardList->Init(m_Shaders[BILLBOARD_SHADER], TREE_BILLBOARD_TEX, 
		0.5f,	// scale
		10,		// numX
		10,		// numY
		2.0f,	// spacing
		14.0f,	// offset pos
		-1.4f	// ypos
	);
	
	return success;
}

bool Renderer::setRenderStates()
{
	// ** These states could differ **
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glFrontFace(GL_CCW);

	return true;
}

bool Renderer::setFrameBuffers()
{
	if (!m_Gbuffer)
	{
		m_Gbuffer = new GBuffer();
	}

	if (!m_Gbuffer->Init())
	{
		WRITE_LOG("Gbuffer load failed", "error");
		return false;
	}

	return true;
}

void Renderer::WindowSizeChanged(int w, int h)
{
	m_CameraPtr->SetAspect(static_cast<float>(w / h));

	if (m_Gbuffer)
	{
		m_Gbuffer->Init();
	}
}

bool Renderer::setLights()
{
	// Dir Light
	m_DirLight.direction = Vec3(0, -1, 0);
	m_DirLight.intensity = Vec3(0.5f);
	m_DirLight.ambient_intensity = 0.1f;

	// Point Light
	m_PointLights.resize(3);

	m_PointLights[0].position = Vec3(2.0f, 3.0f, -2.0f);
	m_PointLights[1].position = Vec3(-2.0f, 3.0f, 2.0f);
	m_PointLights[2].position = Vec3(4.0f, 3.0f, 4.0f);
	m_PointLights[0].intensity = Vec3(1.0f, 0.0f, 0.0f);
	m_PointLights[1].intensity = Vec3(0.0f, 1.0f, 0.0f);
	m_PointLights[2].intensity = Vec3(0.0f, 0.0f, 1.0f);

	for (size_t i = 0; i < m_PointLights.size(); ++i)
	{
		//m_PointLights[i].intensity = Vec3(0.75f);
		m_PointLights[i].ambient_intensity = 0.2f;
		m_PointLights[i].aConstant = ATTEN_CONST;
		m_PointLights[i].aLinear = ATTEN_LIN;
		m_PointLights[i].aQuadratic = ATTEN_QUAD;
	}

	// Spot Light(s)
	m_SpotLight.position = Vec3(0.0f);
	m_SpotLight.direction = Vec3(0.0f, 0.0f, -1.0f);
	m_SpotLight.switched_on = 1;
	m_SpotLight.intensity = Vec3(0.2f, 0.2f, 0.6f);
	m_SpotLight.aConstant = ATTEN_CONST;
	m_SpotLight.aLinear = 0.0174;
	m_SpotLight.aQuadratic = ATTEN_QUAD;
	m_SpotLight.SetAngle(15.0f);

	return true;
}

bool Renderer::SetCamera(BaseCamera* camera)
{
	m_CameraPtr = camera;
	return true;
}

bool Renderer::loadFonts()
{
	m_Font = new Font();
	if (!m_Font->CreateFont("../resources/fonts/cour.ttf", 24))
	{
		WRITE_LOG("FONT LOAD FAIL", "error");
		return false;
	}

	return true;
}

bool Renderer::loadTexture(const std::string& path, size_t key_store, int glTextureIndex)
{
	Image i;
	std::string full_path =  path;
	if (!i.LoadImg(full_path.c_str()))
	{
		WRITE_LOG("Failed to load texture: " + path, "error");
		return false;
	}

	if (m_Textures.find(key_store) != m_Textures.end())
	{
		WRITE_LOG("Tried to use same texture key twice", "error");
		return false;
	}

	Texture* t = new Texture(path, GL_TEXTURE_2D, glTextureIndex);
	m_Textures[key_store] = t;

	if (!t->Create(&i))
	{
		WRITE_LOG("Failed to create texture: " + path, "error");
		return false;
	}

	return true;
}

bool Renderer::loadMesh(const std::string& path, size_t key_store, bool tangents, bool withTextures)
{
	if (m_Meshes.find(key_store) != m_Meshes.end())
	{
		WRITE_LOG("Tried to use same mesh key twice", "error");
		return false;
	}

	Mesh* mesh = new Mesh();
	m_Meshes[key_store] = mesh;
	if (!mesh->Load(path, tangents, withTextures))
	{
		WRITE_LOG("Failed to load mesh: " + path, "error");
		return false;
	}

	return true;
}

bool Renderer::loadTetxures()
{
	bool success = true;

	success &= this->loadTexture("../resources/textures/male_body_low_albedo.tga", MALE_TEX1, GL_TEXTURE0);
	success &= this->loadTexture("../resources/textures/male_body_high_albedo.tga", MALE_TEX2, GL_TEXTURE0);
	success &= this->loadTexture("../resources/textures/dino_diffuse.tga", DINO_TEX, GL_TEXTURE0);
	success &= this->loadTexture("../resources/textures/terrain.tga", WALL_TEX, GL_TEXTURE0);
	success &= this->loadTexture("../resources/textures/bricks/bricks.tga", BRICK_TEX, GL_TEXTURE0);
	success &= this->loadTexture("../resources/textures/bricks/bricks_normal.tga", BRICK_NORM_TEX, GL_TEXTURE2);
	success &= this->loadTexture("../resources/textures/default_normal_map.tga", FAKE_NORMAL_TEX, GL_TEXTURE2);
	success &= this->loadTexture("../resources/textures/billboards/grass.tga", TREE_BILLBOARD_TEX, GL_TEXTURE0);

	success &= this->loadTexture("../resources/textures/terrain/fungus.tga", TERRAIN1_TEX, GL_TEXTURE0);
	success &= this->loadTexture("../resources/textures/terrain/sand_grass.tga", TERRAIN2_TEX, GL_TEXTURE1);
	success &= this->loadTexture("../resources/textures/terrain/rock.tga", TERRAIN3_TEX, GL_TEXTURE2);
	success &= this->loadTexture("../resources/textures/terrain/sand.tga", TERRAIN4_TEX, GL_TEXTURE3);
	success &= this->loadTexture("../resources/textures/terrain/path.tga", TERRAIN5_TEX, GL_TEXTURE4);

	success &= this->loadTexture("../resources/textures/noise.tga", LAVA_NOISE_TEX, GL_TEXTURE0);

	// Load Images for skybox
	Image* images[6];
	Image i0;
	Image i1;
	Image i2;
	Image i3;
	Image i4;
	Image i5;
	if (!i0.LoadImg("../resources/textures/skybox/rightr.tga"))
		return false;
	if (!i1.LoadImg("../resources/textures/skybox/leftr.tga"))
		return false;
	if (!i2.LoadImg("../resources/textures/skybox/topr.tga"))
		return false;
	if (!i3.LoadImg("../resources/textures/skybox/bottomr.tga"))
		return false;
	if (!i4.LoadImg("../resources/textures/skybox/backr.tga"))
		return false;
	if (!i5.LoadImg("../resources/textures/skybox/frontr.tga"))
		return false;

	images[0] = &i0;
	images[1] = &i1;
	images[2] = &i2;
	images[3] = &i3;
	images[4] = &i4;
	images[5] = &i5;

	Texture* cubeMapTex = new Texture("skyboxCubemap", GL_TEXTURE_CUBE_MAP, GL_TEXTURE0);
	m_Textures[SKYBOX_TEX] = cubeMapTex;
	success &= cubeMapTex->Create(images);

	return success;
}

bool Renderer::loadMeshes()
{
	// ---- Load Meshes ----
	bool success = true;

	success &= loadMesh("cube.obj", CUBE_MESH, false, false);
	success &= loadMesh("quad.obj", QUAD_MESH, false, false);
	success &= loadMesh("sphere.obj", SPHERE_MESH, false, false);
	success &= loadMesh("sponza/sponza.obj", SPONZA_MESH, false, true);
	//success &= loadMesh("male.obj", MALE_MESH, !false);
	//success &= loadMesh("dino.obj", DINO_MESH, !false);

	return success;
}

bool Renderer::createMaterials()
{
	// TODO : change this on window size callback
	Vec2 screenSize((float)Screen::Instance()->FrameBufferWidth(), (float)Screen::Instance()->FrameBufferHeight());

	// ---- Font material (Fwd) ----
	{
		Shader font_vert(GL_VERTEX_SHADER);
		Shader font_frag(GL_FRAGMENT_SHADER);
		if (!font_vert.LoadShader("../resources/shaders/font_vs.glsl")) { return false; }
		if (!font_frag.LoadShader("../resources/shaders/font_fs.glsl")) { return false; }
		font_vert.AddAttribute(POS_ATTR);
		std::vector<Shader> shaders;
		shaders.push_back(font_vert);
		shaders.push_back(font_frag);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[FONT_SHADER] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		font_vert.Close();
		font_frag.Close();
	}

	// ---- Skybox (Fwd) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/skybox_vs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/skybox_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);
		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[SKYBOX_SHADER] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		vert.Close();
		frag.Close();

		// One off values
		int texUnit = 0;
		m_Shaders[SKYBOX_SHADER]->Use();
		m_Shaders[SKYBOX_SHADER]->SetUniformValue<int>("cube_sampler", &texUnit);
	}

	// ---- Bill board (Fwd) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader geom(GL_GEOMETRY_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/billboard_vs.glsl")) { return false; }
		if (!geom.LoadShader("../resources/shaders/billboard_gs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/billboard_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(geom);
		shaders.push_back(frag);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[BILLBOARD_SHADER] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		vert.Close();
		geom.Close();
		frag.Close();
	}

	// ---- Terrain (Def) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/terrain_vs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/terrain_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);
		vert.AddAttribute(NORM_ATTR);
		vert.AddAttribute(TEX_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[TERRAIN_SHADER] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		vert.Close();
		frag.Close();

		m_Shaders[TERRAIN_SHADER]->Use();
		for (int i = 0; i < 5; ++i)
		{
			m_Shaders[TERRAIN_SHADER]->SetUniformValue<int>("u_Sampler" + std::to_string(i), &i);
		}
	}

	// ---- Lava Shader (Def) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/deferred/geometry_pass_vs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/lava_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);
		vert.AddAttribute(NORM_ATTR);
		vert.AddAttribute(TEX_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[LAVA_SHADER] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		vert.Close();
		frag.Close();

		m_Shaders[LAVA_SHADER]->Use();
		int sampler = 0;
		m_Shaders[LAVA_SHADER]->SetUniformValue<int>("u_Sampler", &sampler);
		m_Shaders[LAVA_SHADER]->SetUniformValue<Vec2>("u_Resolution", &screenSize);
	}

	// ---- Std Geom Shader (Def) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/deferred/geometry_pass_vs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/deferred/geometry_pass_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);
		vert.AddAttribute(NORM_ATTR);
		vert.AddAttribute(TEX_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[STD_DEF_GEOM_SHADER] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		vert.Close();
		frag.Close();

		m_Shaders[STD_DEF_GEOM_SHADER]->Use();
		int sampler = 0;
		m_Shaders[STD_DEF_GEOM_SHADER]->SetUniformValue<int>("u_ColourMap", &sampler);
	}

	// ---- Std Stencil Shader (Def) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/deferred/stencil_pass_vs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/deferred/stencil_pass_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[STD_DEF_STENCIL_SHADER] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		vert.Close();
		frag.Close();
	}

	// ---- Std Point Light Shader (Def) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/deferred/light_pass_vs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/deferred/point_light_pass_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);
		vert.AddAttribute(NORM_ATTR);
		vert.AddAttribute(TEX_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[STD_DEF_PNT_LIGHT_SHADER] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		vert.Close();
		frag.Close();

		int p = (int)GBuffer::TexTypes::Position;
		int d = (int)GBuffer::TexTypes::Diffuse;
		int n = (int)GBuffer::TexTypes::Normal;
		float matSpec = 0.1f;

		m_Shaders[STD_DEF_PNT_LIGHT_SHADER]->Use();
		m_Shaders[STD_DEF_PNT_LIGHT_SHADER]->SetUniformValue<int>("u_PositionMap", &p);
		m_Shaders[STD_DEF_PNT_LIGHT_SHADER]->SetUniformValue<int>("u_ColourMap", &d);
		m_Shaders[STD_DEF_PNT_LIGHT_SHADER]->SetUniformValue<int>("u_NormalMap", &n);
		m_Shaders[STD_DEF_PNT_LIGHT_SHADER]->SetUniformValue<Vec2>("u_ScreenSize", &screenSize);
		m_Shaders[STD_DEF_PNT_LIGHT_SHADER]->SetUniformValue<float>("u_MatSpecularIntensity", &matSpec);
		m_Shaders[STD_DEF_PNT_LIGHT_SHADER]->SetUniformValue<float>("u_SpecularPower", &matSpec);
	}

	// ---- Std Dir Light Shader (Def) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/deferred/light_pass_vs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/deferred/dir_light_pass_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);
		vert.AddAttribute(NORM_ATTR);
		vert.AddAttribute(TEX_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		vert.Close();
		frag.Close();

		int p = (int)GBuffer::TexTypes::Position;
		int d = (int)GBuffer::TexTypes::Diffuse;
		int n = (int)GBuffer::TexTypes::Normal;
		float matSpec = 0.2f;

		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->Use();
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<Vec3>("u_DirectionalLight.Base.Color", &m_DirLight.intensity);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<Vec3>("u_DirectionalLight.Direction", &m_DirLight.direction);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<float>("u_DirectionalLight.Base.AmbientIntensity", &m_DirLight.ambient_intensity);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<float>("u_DirectionalLight.Base.DiffuseIntensity", &m_DirLight.ambient_intensity);


		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<int>("u_PositionMap", &p);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<int>("u_ColourMap", &d);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<int>("u_NormalMap", &n);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<Vec2>("u_ScreenSize", &screenSize);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<float>("u_MatSpecularIntensity", &matSpec);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<float>("u_SpecularPower", &matSpec);
	}

	// New fwd lighting stuff
	{
		Shader geom_vs(GL_VERTEX_SHADER);
		Shader fwd_fs(GL_FRAGMENT_SHADER);
		//Shader lights_fs(GL_FRAGMENT_SHADER);
		//Shader refl_fs(GL_FRAGMENT_SHADER);

		if (!geom_vs.LoadShader("../resources/shaders/new_lights/geom_vs.glsl"))
		{
			WRITE_LOG("Geom shader failed compile", "error");
			return false;
		}

		if (!fwd_fs.LoadShader("../resources/shaders/new_lights/forward_render_fs.glsl"))
		{
			WRITE_LOG("Forward render failed compile", "error");
			return false;
		}

		geom_vs.AddAttribute(POS_ATTR);
		geom_vs.AddAttribute(NORM_ATTR);
		geom_vs.AddAttribute(TEX_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(geom_vs);
		shaders.push_back(fwd_fs);

		ShaderProgram* shaderProg = new ShaderProgram();
		m_Shaders[STD_FWD_LIGHTING] = shaderProg;
		if (!shaderProg->CreateProgram(shaders, "frag_colour", 0)) return false;
		geom_vs.Close();
		fwd_fs.Close();

		m_Shaders[STD_FWD_LIGHTING]->Use();

		int sampler = 0;

		// Set constants
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<int>("u_sampler", &sampler);
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<Vec3>("u_ambience", &AMBIENT_LIGHT);

		// Dir light
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<Vec3>("u_dir_light.intensity", &m_DirLight.intensity);
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<Vec3>("u_dir_light.direction", &m_DirLight.direction);
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<float>("u_dir_light.ambient_intensity", &m_DirLight.ambient_intensity);

		// Spot Light
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<Vec3>("u_spot_light.position", &m_SpotLight.position);
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<Vec3>("u_spot_light.direction", &m_SpotLight.direction);
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<Vec3>("u_spot_light.intensity", &m_SpotLight.intensity);
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<float>("u_spot_light.coneAngle",&m_SpotLight.GetAngle());
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<float>("u_spot_light.aConstant", &m_SpotLight.aConstant);
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<float>("u_spot_light.aLinear", &m_SpotLight.aLinear);
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<float>("u_spot_light.aQuadratic", &m_SpotLight.aQuadratic);
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<int>("u_spot_light.switched_on", &m_SpotLight.switched_on);

		// Point Light
		int num_points = static_cast<int>(m_PointLights.size());
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<int>("u_num_point_lights", &num_points);
		for (int i = 0; i < num_points; ++i)
		{
			m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<Vec3>("u_point_light[" + std::to_string(i) +  "].position", &m_PointLights[i].position);
			m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<Vec3>("u_point_light[" + std::to_string(i) +  "].intensity", &m_PointLights[i].intensity);
			m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<float>("u_point_light[" + std::to_string(i) + "].ambient_intensity", &m_PointLights[i].ambient_intensity);
			m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<float>("u_point_light[" + std::to_string(i) + "].aConstant", &m_PointLights[i].aConstant);
			m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<float>("u_point_light[" + std::to_string(i) + "].aLinear", &m_PointLights[i].aLinear);
			m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<float>("u_point_light[" + std::to_string(i) + "].aQuadratic", &m_PointLights[i].aQuadratic);
		}

		/*
		if (!lights_fs.LoadShader("../resources/shaders/new_lights/lights_fs.glsl"))
		{
			WRITE_LOG("lights shader failed compile", "error");
			return false;
		}

		if (!refl_fs.LoadShader("../resources/shaders/new_lights/reflection_model_fs.glsl"))
		{
			WRITE_LOG("Reflection shader failed compile", "error");
			return false;
		}
		*/
	}

	return true;
}

bool Renderer::CreateTexture(size_t& out, const std::string& path, int glTexId)
{
	size_t key = m_Textures.size();
	bool found_empty = false;

	while (!found_empty)
	{
		if (m_Textures.find(key) != m_Textures.end())
		{
			++key;
		}
		else
		{
			found_empty = true;
		}
	}

	if (this->loadTexture(path, key, glTexId))
	{
		out = key;
		return true;
	}

	return false;
}

Texture* Renderer::GetTexture(size_t index)
{
	return index < m_Textures.size() ? m_Textures[index] : nullptr;
}

size_t Renderer::GetNumTextures() const
{
	return m_Textures.size();
}

size_t Renderer::GetNumSubMeshesInMesh(size_t meshIndex)
{
	auto r = m_Meshes.find(meshIndex);
	if (r != m_Meshes.end())
	{
		return r->second->m_SubMeshes.size();
	}
	else
	{
		WRITE_LOG("error invalid mesh", "error");
		return 0;
	}
}

void Renderer::RenderMesh(Mesh* thisMesh)
{
	glBindVertexArray(thisMesh->m_VAO);

	int meshIndex = 0;
	for (std::vector<SubMesh>::iterator j = thisMesh->m_SubMeshes.begin(); j != thisMesh->m_SubMeshes.end(); j++)
	{
		SubMesh subMesh = (*j);

		if (subMesh.NumIndices > 0)
		{
			glDrawElementsBaseVertex(GL_TRIANGLES,
				subMesh.NumIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * subMesh.BaseIndex),
				subMesh.BaseVertex);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, subMesh.NumVertices);
		}
	}

	glBindVertexArray(0);
}

void Renderer::RenderMesh(MeshRenderer* meshInstance)
{
	Mesh* thisMesh = m_Meshes[meshInstance->MeshIndex];
	if (!thisMesh)
		return;

	glBindVertexArray(thisMesh->m_VAO);

	int meshIndex = 0;
	for (std::vector<SubMesh>::iterator j = thisMesh->m_SubMeshes.begin(); j != thisMesh->m_SubMeshes.end(); j++)
	{
		SubMesh subMesh = (*j);

		if (thisMesh->HasTextures())
		{
			const unsigned int MaterialIndex = subMesh.MaterialIndex;
			if(thisMesh->m_Textures[MaterialIndex])
			{
				thisMesh->m_Textures[MaterialIndex]->Bind();
			}
		}
		else
		{
			// Bind the textures for this mesh instance
			for (auto tex = meshInstance->m_SubMeshTextures[meshIndex].begin();
				tex != meshInstance->m_SubMeshTextures[meshIndex].end(); ++tex)
			{
				m_Textures[meshInstance->m_TextureHandles[(*tex)]]->Bind();
			}
		}

		if (subMesh.NumIndices > 0)
		{
			glDrawElementsBaseVertex(GL_TRIANGLES,
				subMesh.NumIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * subMesh.BaseIndex),
				subMesh.BaseVertex);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, subMesh.NumVertices);
		}

		++meshIndex;
	}

	glBindVertexArray(0);
}

void Renderer::RenderSkybox(BaseCamera* cam)
{
	if(m_DeferredRender)
		glEnable(GL_DEPTH_TEST);
	
	// Use skybox material
	m_Shaders[SKYBOX_SHADER]->Use();

	SkyboxSettings* sb = cam->SkyBoxParams();
	if (!sb)
		return;

	// States
	GLint oldCullMode, oldDepthFunc;
	glGetIntegerv(GL_CULL_FACE_MODE, &oldCullMode);
	glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	Mat4 model = glm::translate(IDENTITY, cam->Position())
		* glm::scale(IDENTITY, Vec3(sb->scale));

	m_Shaders[SKYBOX_SHADER]->SetUniformValue<Mat4>("wvp_xform", &(cam->Projection() * cam->View() * model));

	// Render mesh with texture here, THIS SHOULDNT BE HERE
	glBindVertexArray(m_Meshes[CUBE_MESH]->m_VAO);
	Texture* t = m_Textures[sb->textureIndex];
	if (t) t->Bind();

	for (std::vector<SubMesh>::iterator i = m_Meshes[CUBE_MESH]->m_SubMeshes.begin();
		i != m_Meshes[CUBE_MESH]->m_SubMeshes.end(); i++)
	{
		SubMesh subMesh = (*i);

		if (subMesh.NumIndices > 0)
		{
			glDrawElementsBaseVertex(GL_TRIANGLES,
				subMesh.NumIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * subMesh.BaseIndex),
				subMesh.BaseVertex);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, subMesh.NumVertices);
		}
	}

	glBindVertexArray(0);

	glCullFace(oldCullMode);
	glDepthFunc(oldDepthFunc);

	if(m_DeferredRender)
		glDisable(GL_DEPTH_TEST);
}

void Renderer::Render(std::vector<GameObject*>& gameObjects)
{
	// Temp
	if (Mouse::Instance()->LMB)
		DIRECTION_ANGLE += 1.0f;
	if (Mouse::Instance()->RMB)
	{
		m_SpotLight.ToggleLight();

		m_Shaders[STD_FWD_LIGHTING]->Use();
		m_Shaders[STD_FWD_LIGHTING]->SetUniformValue<int>("u_spot_light.switched_on", &m_SpotLight.switched_on);
	}


	m_Query.Start();

	if (m_DeferredRender)
		DeferredRender(gameObjects);
	else
		ForwardRender(gameObjects);

	m_Query.End();

	if (++m_Frames > 2)
	{
		m_QueryTime = m_Query.Result(false);
		m_Frames = 0;
	}

	// -- Render Text ----
	RenderText("Frm Time: " + util::to_str(GetFrameTime(TimeMeasure::Seconds)), 8, Screen::Instance()->FrameBufferHeight() - 16.0f, FontAlign::Left, Colour::Red());

}

void Renderer::ForwardRender(std::vector<GameObject*>& gameObjects)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!m_CameraPtr)
		// Should log if that hasn't been set
		return;

	if (m_CameraPtr->HasSkybox())
	{
		this->RenderSkybox(m_CameraPtr);
	}

	Mat4 proj_xform = m_CameraPtr->Projection();
	Mat4 view_xform = m_CameraPtr->View();

	ShaderProgram* sp = m_Shaders[STD_FWD_LIGHTING]; //m_Shaders[mr->m_ShaderIndex];
	if (sp)
	{
		sp->Use();

		// Set Matrices
		sp->SetUniformValue<Mat4>("u_proj_xform", &(proj_xform));
		sp->SetUniformValue<Mat4>("u_view_xform", &(view_xform));

		// Update Cam spot light
		Vec3 spot_dir = m_CameraPtr->Forward();
		Vec3 spot_pos = m_CameraPtr->Position();
		sp->SetUniformValue<Vec3>("u_spot_light.direction", &spot_dir);
		sp->SetUniformValue<Vec3>("u_spot_light.position", &spot_pos);

		// Update Point Light
		float light_val_x = sinf(Time::ElapsedTime()) * 8.0f;
		float light_val_z = cosf(Time::ElapsedTime()) * 8.0f;
		for (int i = 0; i < 1; ++i)
		{
			m_PointLights[i].position = 
				Vec3(light_val_x, 
					m_PointLights[i].position.y, 
					light_val_z);

			sp->SetUniformValue<Vec3>("u_point_light["+std::to_string(i)+"].position", 
				&m_PointLights[i].position);
		}
		
		// Update Dir Light
		m_DirLight.direction = Vec3(cos(DIRECTION_ANGLE * 3.1415 / 180.0) * 70, sin(DIRECTION_ANGLE * 3.1415 / 180.0) * 70, 0.0);
		sp->SetUniformValue<Vec3>("u_dir_light.direction", &-glm::normalize(m_DirLight.direction));
	}

	// Render Mesh Renderers
	for (auto i = gameObjects.begin(); i != gameObjects.end(); ++i)
	{
		Transform* t = (*i)->GetComponent<Transform>();
		MeshRenderer* mr = (*i)->GetComponent<MeshRenderer>();

		if (!t || !mr)
			continue;

		if (sp)
		{
			sp->SetUniformValue<Mat4>("u_world_xform", &(t->GetModelXform()));

			// Render Mesh here
			this->RenderMesh(mr);
		}
	}
}

void Renderer::DeferredRender(std::vector<GameObject*>& gameObjects)
{
	m_Gbuffer->StartFrame();

	// Geom Pass
	{
		// Skybox
		if (m_CameraPtr->HasSkybox())
			RenderSkybox(m_CameraPtr);

		// Terrain
		//m_TreeBillboardList->Render(this, m_Camera->Projection() * m_Camera->View(), m_Camera->Position(), m_Camera->Right());

		// -- Deferred Geom pass ---
		m_Gbuffer->BindForGeomPass();

		// Set GL States
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// RenderTerrain and Billboards
		m_Terrain->Render(this, m_CameraPtr, Vec3(1.0f));

		Mat4 ProjViewXform = m_CameraPtr->Projection() * m_CameraPtr->View();

		// Render Mesh Renderers
		for (auto i = gameObjects.begin(); i != gameObjects.end(); ++i)
		{
			Transform* t = (*i)->GetComponent<Transform>();
			MeshRenderer* mr = (*i)->GetComponent<MeshRenderer>();

			if (!t || !mr)
				continue;

			ShaderProgram* sp = m_Shaders[mr->m_ShaderIndex];
			if (sp)
			{
				sp->Use();
				sp->SetUniformValue<Mat4>("u_WVP", &(ProjViewXform * t->GetModelXform()));
				sp->SetUniformValue<Mat4>("u_World", &(t->GetModelXform()));

				// This is shit
				float elapsed = Time::ElapsedTime();
				sp->SetUniformValue<float>("u_GlobalTime", &elapsed);

				// Render Mesh here
				this->RenderMesh(mr);
			}
		}

		glDepthMask(GL_FALSE);
	}

	// Stencil and Point Lights
	{
		glEnable(GL_STENCIL_TEST);
		
		for (int i = 0; i < m_PointLights.size(); ++i)
		{
			// Stencil
			{
				m_Shaders[STD_DEF_STENCIL_SHADER]->Use();

				// Disable color/depth write and enable stencil
				m_Gbuffer->BindForStencilPass();
				glEnable(GL_DEPTH_TEST);

				glDisable(GL_CULL_FACE);

				glClear(GL_STENCIL_BUFFER_BIT);

				// We need the stencil test to be enabled but we want it to succeed always. Only the depth test matters.
				glStencilFunc(GL_ALWAYS, 0, 0);

				glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
				glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

				Mat4 LIGHT_TRANS = glm::translate(Mat4(1.0f), m_PointLights[i].position) * glm::scale(Mat4(1.0f), Vec3(CalcPointLightBSphere(m_PointLights[i])));
				m_Shaders[STD_DEF_STENCIL_SHADER]->SetUniformValue<Mat4>("u_WVP", &(m_CameraPtr->Projection() * m_CameraPtr->View() * LIGHT_TRANS));
				this->RenderMesh(m_Meshes[SPHERE_MESH]);
			}

			// Point Light
			{
				m_Gbuffer->BindForLightPass();

				ShaderProgram* sp = m_Shaders[STD_DEF_PNT_LIGHT_SHADER];
				sp->Use();
				sp->SetUniformValue<Vec3>("u_EyeWorldPos", &m_CameraPtr->Position());

				glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

				glDisable(GL_DEPTH_TEST);
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_ONE, GL_ONE);

				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);

				Mat4 LIGHT_TRANS = glm::translate(Mat4(1.0f), m_PointLights[i].position) * glm::scale(Mat4(1.0f), Vec3((CalcPointLightBSphere(m_PointLights[i]))));

				// Set PointLight
				sp->SetUniformValue<Vec3>("u_PointLight.Base.Color", &m_PointLights[i].intensity);
				sp->SetUniformValue<Vec3>("u_PointLight.Position", &m_PointLights[i].position);
				sp->SetUniformValue<float>("u_PointLight.Base.AmbientIntensity", &m_PointLights[i].ambient_intensity);
				sp->SetUniformValue<float>("u_PointLight.Atten.Constant", &m_PointLights[i].aConstant);
				sp->SetUniformValue<float>("u_PointLight.Atten.Linear", &m_PointLights[i].aLinear);
				sp->SetUniformValue<float>("u_PointLight.Atten.Exp", &m_PointLights[i].aQuadratic);
				sp->SetUniformValue<Mat4>("u_WVP", &(m_CameraPtr->Projection() * m_CameraPtr->View() * LIGHT_TRANS));

				this->RenderMesh(m_Meshes[SPHERE_MESH]);

				glCullFace(GL_BACK);
				glDisable(GL_BLEND);
			}
		}
		
		glDisable(GL_STENCIL_TEST);
	}

	// Dir Light Pass
	{
		glDisable(GL_CULL_FACE);
		m_Gbuffer->BindForLightPass();
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->Use();
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<Vec3>("u_EyeWorldPos", &m_CameraPtr->Position());
		// Should only set once
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<Mat4>("u_WVP", &(Mat4(1.0f)));

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		this->RenderMesh(m_Meshes[QUAD_MESH]);
		glDisable(GL_BLEND);
	}

	// Final Pass
	{
		int width = Screen::Instance()->FrameBufferWidth();
		int height = Screen::Instance()->FrameBufferHeight();

		m_Gbuffer->BindForFinalPass();

		glBlitFramebuffer(0, 0, width, height,
			0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
}

void Renderer::RenderText(const std::string& txt, float x, float y, FontAlign fa, const Colour& colour)
{
	if (!m_Font)
		return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/*
	if (norm)
	{
		x *= 0.5f;
		y *= 0.5f;
		x += 0.5f;
		y += 0.5f;
		x *= (float)Window::FrameBufferWidth();
		y *= (float)Window::FrameBufferHeight();
	}
	*/

	// Activate corresponding render state	
	m_Shaders[FONT_SHADER]->Use();

	Mat4 projection = glm::ortho(0.0f, (float)Screen::Instance()->FrameBufferWidth(),
		0.0f,
		(float)Screen::Instance()->FrameBufferHeight());

	m_Shaders[FONT_SHADER]->SetUniformValue<Mat4>("proj_xform", &projection);
	m_Shaders[FONT_SHADER]->SetUniformValue<Vec4>("text_colour", &colour.Normalize());

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(m_Font->m_Vao);

	// Iterate through all characters
	std::string::const_iterator i;
	for (i = txt.begin(); i != txt.end(); ++i)
	{
		Character ch = m_Font->m_Characters[(*i)];

		float xpos = x + ch.bearingX;
		float ypos = y - (ch.sizeY - ch.bearingY);

		float w = (float)ch.sizeX;
		float h = (float)ch.sizeY;

		if (fa == FontAlign::Centre)
		{
			xpos -= ((ch.advance >> 6)) * (txt.size() / 2);
		}
		else if (fa == FontAlign::Right)
		{
			xpos -= ((ch.advance >> 6)) * (txt.size());
		}

		// Update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_Font->m_Vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6); // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);
}

float Renderer::GetFrameTime(TimeMeasure tm)
{
	switch (tm)
	{
	case TimeMeasure::NanoSeconds:
		return m_QueryTime;
	case TimeMeasure::MilliSeconds:
		return m_QueryTime / 1000000.f;
	case TimeMeasure::Seconds:
		return m_QueryTime / 100'000'000'0.f;
	}

	return 0.0f;
}

void Renderer::Close()
{
	m_Query.Clean();

	// Clear meshes
	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		SAFE_DELETE(m_Meshes[i]);
	}
	m_Meshes.clear();
	
	for (size_t i = 0; i < m_Shaders.size(); ++i)
	{
		SAFE_CLOSE(m_Shaders[i]);
	}
	m_Shaders.clear();

	// Clear Textures
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		SAFE_DELETE(m_Textures[i]);
	}
	m_Textures.clear();

	SAFE_DELETE(m_TreeBillboardList);
	SAFE_DELETE(m_Terrain);

	// Other objects
	SAFE_CLOSE(m_Font);
	SAFE_DELETE(m_Gbuffer);
}


//------------------------
/*
Renderer::Renderer() :
	m_Meshes(),
	m_Textures(),
	//m_SkyboxMesh(nullptr),
	m_LavaTestMesh(nullptr),
	m_Font(nullptr),
	m_CameaObj(nullptr),
	m_Camera(nullptr),
	m_LightCameaObj(nullptr),
	m_LightCamera(nullptr),
	m_ShadowFBO(nullptr),
	m_TreeBillboardList(nullptr),
	m_Terrain(nullptr),

	m_FontMaterial(nullptr),
	m_LightMaterial(nullptr),
	m_DiffuseMaterial(nullptr),
	m_SkyBoxMaterial(nullptr),
	m_ShadowMaterial(nullptr),
	m_BillboardMaterial(nullptr),
	m_TerrainMaterial(nullptr),
	m_LavaMaterial(nullptr),

	m_DirectionalLight()
{
}

bool Renderer::Init()
{
	bool success = true;

	success &= setRenderStates();
	success &= setFrameBuffers();
	success &= setLights();
	success &= setCamera();
	success &= loadFonts();
	success &= loadTetxures();
	success &= loadMeshes();
	success &= createMaterials();

	std::vector<Vec3> billboardPositions;
	// Create Terrain
	m_Terrain = new Terrain();

	unsigned int textures[5] = { TERRAIN1_TEX, TERRAIN2_TEX, TERRAIN3_TEX, TERRAIN4_TEX, TERRAIN5_TEX };
	success &= m_Terrain->LoadFromHeightMapWithBillboards(
		"../resources/textures/terrain/heightmap.tga", 
		m_TerrainMaterial,
		textures,
		Vec3(200, 30, 200),
		billboardPositions,
		500
	);

	// Need material and textures for bill board creation, which again I am not too happy with
	m_TreeBillboardList = new BillboardList();
	success &= m_TreeBillboardList->InitWithPositions(m_BillboardMaterial, TREE_BILLBOARD_TEX, 0.5f, billboardPositions);
	//
	//	m_TreeBillboardList->Init(m_BillboardMaterial, TREE_BILLBOARD_TEX, 
	//	0.5f,	// scale
	//	10,		// numX
	//	10,		// numY
	//	2.0f,	// spacing
	//	14.0f,	// offset pos
	//	-1.4f	// ypos
	//);
	

	return success;
}

bool Renderer::setRenderStates()
{
	// ** These states could differ **
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS);
	
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glFrontFace(GL_CCW);

	return true;
}

bool Renderer::setFrameBuffers()
{
	if (DO_SHADOWS)
	{
		m_ShadowFBO = new ShadowFrameBuffer();
		if (!m_ShadowFBO->Init(Screen::Instance()->FrameBufferWidth(), Screen::Instance()->FrameBufferHeight()))
		{
			return false;
		}
	}

	return true;
}

void Renderer::WindowSizeChanged(int w, int h)
{
	m_Camera->SetAspect(static_cast<float>(w / h));

	// TODO : Put this in a function
	m_LightCamera->SetAspect(static_cast<float>(w / h));
	m_LightCamera->Update();

	// Agh this sucks
	if (m_ShadowFBO && DO_SHADOWS)
	{
		m_ShadowFBO->Init(w, h);
	}
}

bool Renderer::setLights()
{
	// Dir Light
	m_DirectionalLight.Color = Vec3(1.0f, 1.0f, 1.0f);
	m_DirectionalLight.AmbientIntensity = 0.4f;
	m_DirectionalLight.DiffuseIntensity = 0.01f;
	m_DirectionalLight.Direction = Vec3(1.0f, -1.0f, 0.0f);

	
	// Point Lights
	//m_PointLights[0].Color = Vec3(1.0f, 1.0f, 1.0f);
	//m_PointLights[0].DiffuseIntensity = 0.5f;
	//m_PointLights[0].Position = Vec3(-7.0f, 5.0f, -7.0f);
	//m_PointLights[0].Attenuation.Linear = 0.1f;

	//m_PointLights[1].Color = Vec3(1.0f, 1.0f, 1.0f);
	//m_PointLights[1].DiffuseIntensity = 0.5f;
	//m_PointLights[1].Attenuation.Linear = 0.1f;
	//m_PointLights[1].Position = Vec3(7.0f, 5.0f, 7.0f);

	// Spot Lights
	m_SpotLights[0].DiffuseIntensity = 0.9f;
	m_SpotLights[0].Color = Vec3(1.0f, 1.0f, 1.0f);
	m_SpotLights[0].Attenuation.Linear = 0.1f;
	m_SpotLights[0].Cutoff = Maths::ToRadians(30.0f);
	m_SpotLights[0].Direction = Vec3(-0.1f, -1, 0.0f);
	m_SpotLights[0].Position = Vec3(2, 5, 5.0f);
	m_SpotLights[0].Direction = Vec3(-0.5f, -1.0f, 0.0f)- m_SpotLights[0].Position;

	//m_SpotLights[1].DiffuseIntensity = 0.9f;
	//m_SpotLights[1].Color = Vec3(1.0f, 1.0f, 1.0f);
	//m_SpotLights[1].Attenuation.Linear = 0.1f;
	//m_SpotLights[1].Cutoff = Maths::ToRadians(100.0f);
	//m_SpotLights[1].Direction = Vec3(0.1f, -1, -0.1f);

	return true;
}

bool Renderer::setCamera()
{
	// Camera
	m_CameaObj = new GameObject();
	m_Camera = m_CameaObj->AddComponent<FlyCamera>();

	m_Camera->Start();
	m_Camera->Init(
		CamType::Perspective,
		Vec3(0, 1.0f, 4.0f),
		Vec3(0.0f, 1.0f, 0.0f),
		Vec3(1.0f, 0.0f, 0.0f),
		Vec3(0.0f, 0.0f, -1.0f),
		45.0f,
		static_cast<float>(Screen::Instance()->ScreenWidth() / Screen::Instance()->ScreenHeight()),
		0.1f,
		200.0f
	);

	m_Camera->AddSkybox(30.0f, SKYBOX_TEX);

	// Light Camera
	m_LightCameaObj = new GameObject();
	m_LightCamera = m_LightCameaObj->AddComponent<BaseCamera>();
	m_LightCamera->Start();
	m_LightCamera->Init(
		CamType::Perspective,
		m_SpotLights[0].Position,
		Vec3(0.0f, 1.0f, 0.0f),
		Vec3(1.0f, 0.0f, 0.0f),
		m_SpotLights[0].Direction,
		45.0f,
		static_cast<float>(Screen::Instance()->ScreenWidth() / Screen::Instance()->ScreenHeight()),
		0.1f,
		200.0f
	);

	m_LightCamera->Update();

	return true;
}

bool Renderer::loadFonts()
{
	m_Font = new Font();
	if (!m_Font->CreateFont("../resources/fonts/cour.ttf", 24))
	{
		WRITE_LOG("FONT LOAD FAIL", "error");
		return false;
	}

	return true;
}

bool Renderer::loadTexture(const std::string& path, size_t key_store, int glTextureIndex)
{
	Image i;
	std::string full_path = "../resources/textures/" + path;
	if (!i.LoadImg(full_path.c_str()))
	{
		WRITE_LOG("Failed to load texture: " + path, "error");
		return false;
	}

	if (m_Textures.find(key_store) != m_Textures.end())
	{
		WRITE_LOG("Tried to use same texture key twice", "error");
		return false;
	}

	Texture* t = new Texture(path, GL_TEXTURE_2D, glTextureIndex);
	m_Textures[key_store] = t;

	if (!t->Create(&i))
	{
		WRITE_LOG("Failed to create texture: " + path, "error");
		return false;
	}
	
	return true;
}

bool Renderer::loadMesh(const std::string& path, size_t key_store, bool tangents)
{
	if (m_Meshes.find(key_store) != m_Meshes.end())
	{
		WRITE_LOG("Tried to use same mesh key twice", "error");
		return false;
	}

	Mesh* mesh = new Mesh();
	m_Meshes[key_store] = mesh;
	if (!mesh->Load(path, tangents))
	{
		WRITE_LOG("Failed to load mesh: " + path, "error");
		return false;
	}

	return true;
}

bool Renderer::loadTetxures()
{
	bool success = true;

	success &= this->loadTexture("male_body_low_albedo.tga", MALE_TEX1, GL_TEXTURE0);
	success &= this->loadTexture("male_body_high_albedo.tga", MALE_TEX2, GL_TEXTURE0);
	success &= this->loadTexture("dino_diffuse.tga", DINO_TEX, GL_TEXTURE0);
	success &= this->loadTexture("terrain.tga", WALL_TEX, GL_TEXTURE0);
	success &= this->loadTexture("bricks/bricks.tga", BRICK_TEX, GL_TEXTURE0);
	success &= this->loadTexture("bricks/bricks_normal.tga", BRICK_NORM_TEX, GL_TEXTURE2);
	success &= this->loadTexture("default_normal_map.tga", FAKE_NORMAL_TEX, GL_TEXTURE2);
	success &= this->loadTexture("billboards/grass.tga", TREE_BILLBOARD_TEX, GL_TEXTURE0);

	success &= this->loadTexture("terrain/fungus.tga", TERRAIN1_TEX, GL_TEXTURE0);
	success &= this->loadTexture("terrain/sand_grass.tga", TERRAIN2_TEX, GL_TEXTURE1);
	success &= this->loadTexture("terrain/rock.tga", TERRAIN3_TEX, GL_TEXTURE2);
	success &= this->loadTexture("terrain/sand.tga", TERRAIN4_TEX, GL_TEXTURE3);
	success &= this->loadTexture("terrain/path.tga", TERRAIN5_TEX, GL_TEXTURE4);

	success &= this->loadTexture("noise.tga", LAVA_NOISE_TEX, GL_TEXTURE0);


	// Load Images for skybox
	Image* images[6];
	Image i0;
	Image i1;
	Image i2;
	Image i3;
	Image i4;
	Image i5;
	if (!i0.LoadImg("../resources/textures/skybox/rightr.tga"))
		return false;
	if (!i1.LoadImg("../resources/textures/skybox/leftr.tga"))
		return false;
	if (!i2.LoadImg("../resources/textures/skybox/topr.tga"))
		return false;
	if (!i3.LoadImg("../resources/textures/skybox/bottomr.tga"))
		return false;
	if (!i4.LoadImg("../resources/textures/skybox/backr.tga"))
		return false;
	if (!i5.LoadImg("../resources/textures/skybox/frontr.tga"))
		return false;

	images[0] = &i0;
	images[1] = &i1;
	images[2] = &i2;
	images[3] = &i3;
	images[4] = &i4;
	images[5] = &i5;

	Texture* cubeMapTex = new Texture("skyboxCubemap", GL_TEXTURE_CUBE_MAP, GL_TEXTURE0);
	m_Textures[SKYBOX_TEX] = cubeMapTex;
	success &= cubeMapTex->Create(images);

	return success;
}

bool Renderer::loadMeshes()
{
	// ---- Load Meshes ----
	bool success = true;

	success &= loadMesh("cube.obj", CUBE_MESH, !false);
	success &= loadMesh("quad.obj", QUAD_MESH, !false);
	success &= loadMesh("sphere.obj", SPHERE_MESH, !false);
	success &= loadMesh("male.obj", MALE_MESH, !false);
	success &= loadMesh("dino.obj", DINO_MESH, !false);

	//const Mat4 FLOOR_XFORM = glm::translate(Mat4(1.0f), Vec3(0.0f, -2.0f, -3.0f))  *  glm::scale(Mat4(1.0f), Vec3(30.0f, 0.5f, 30.0f));
	//const Mat4 LAVA_XFORM = glm::translate(Mat4(1.0f), Vec3(0.0f, 4.0f, 0.0f)) *  glm::scale(Mat4(1.0f), Vec3(400.0f, 2.0f, 400.0f));

	// Do Mesh instances or game objects here
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
	m_GameObjects.push_back(male);

	GameObject* dino = new GameObject();
	Transform* dinoT = dino->AddComponent<Transform>();
	dinoT->SetPosition(Vec3(1.8f, -1.5f, -1.0f));
	dinoT->SetScale(Vec3(0.05f));
	MeshRenderer* dinoMr = dino->AddComponent<MeshRenderer>();
	dinoMr->SetMesh(DINO_MESH, m_Meshes[DINO_MESH]->m_SubMeshes.size());
	dinoMr->AddTexture(DINO_TEX);
	dinoMr->AddTexture(FAKE_NORMAL_TEX);
	m_GameObjects.push_back(dino);

	GameObject* cube1 = new GameObject();
	Transform* cubet = cube1->AddComponent<Transform>();
	cubet->SetPosition(Vec3(0.0f, 1.0f, -10.0f));
	cubet->SetScale(Vec3(1.0f));
	MeshRenderer* cube1Mr = cube1->AddComponent<MeshRenderer>();
	cube1Mr->SetMesh(CUBE_MESH, m_Meshes[CUBE_MESH]->m_SubMeshes.size());
	cube1Mr->AddTexture(BRICK_TEX);
	cube1Mr->AddTexture(BRICK_NORM_TEX);
	m_GameObjects.push_back(cube1);

	GameObject* cube2 = new GameObject();
	Transform* cube2t = cube2->AddComponent<Transform>();
	cube2t->SetPosition(Vec3(-2.0f, 0.0f, 0.0f));
	cube2t->SetScale(Vec3(1.0f));
	MeshRenderer* cube2Mr = cube2->AddComponent<MeshRenderer>();
	cube2Mr->SetMesh(CUBE_MESH, m_Meshes[CUBE_MESH]->m_SubMeshes.size());
	cube2Mr->AddTexture(BRICK_TEX);
	cube2Mr->AddTexture(FAKE_NORMAL_TEX);
	m_GameObjects.push_back(cube2);

	GameObject* floor = new GameObject();
	Transform* ft = floor->AddComponent<Transform>();
	ft->SetPosition(Vec3(0, -2.0f, -3.0f));
	ft->SetScale(Vec3(30, 0.5f, 30.0f));
	MeshRenderer* fmr = floor->AddComponent<MeshRenderer>();
	fmr->SetMesh(CUBE_MESH, m_Meshes[CUBE_MESH]->m_SubMeshes.size());
	fmr->AddTexture(WALL_TEX);
	fmr->AddTexture(FAKE_NORMAL_TEX);
	m_GameObjects.push_back(floor);

	// This is separate (for now)
	m_LavaTestMesh = new MeshRenderer(nullptr);
	m_LavaTestMesh->SetMesh(CUBE_MESH, m_Meshes[CUBE_MESH]->m_SubMeshes.size());
	m_LavaTestMesh->AddTexture(LAVA_NOISE_TEX);

	return success;
}

bool Renderer::createMaterials()
{
	// ---- Standard Diffuse Mat ----
	m_DiffuseMaterial = new BasicDiffuseTechnique();
	if (!m_DiffuseMaterial->Init())
	{
		WRITE_LOG("Failed to create diffuse mat", "error");
		return false;
	}

	// ---- Font material -----
	m_FontMaterial = new FontTechnique();
	if (!m_FontMaterial->Init())
	{
		WRITE_LOG("Failed to create font mat", "error");
		return false;
	}

	// Light material for forward rendering
	m_LightMaterial = new LightTechnique();
	if (!m_LightMaterial->Init())
	{
		WRITE_LOG("Light effect error", "error");
		return false;
	}
	// Assumes lights are set first here
	m_LightMaterial->Use();
	m_LightMaterial->setTextureUnit(0);
	m_LightMaterial->setShadowSampler(1);
	m_LightMaterial->setNormalSampler(2);
	m_LightMaterial->setDirectionalLight(m_DirectionalLight);
	m_LightMaterial->setMatSpecularIntensity(1.0f);
	m_LightMaterial->setMatSpecularPower(2.0f);

	// ---- Skybox material ----
	m_SkyBoxMaterial = new SkyboxTechnique();
	if (!m_SkyBoxMaterial->Init())
	{
		WRITE_LOG("Failed to create skybox mat", "error");
		return false;
	}

	// ---- Shadow material ----
	m_ShadowMaterial = new ShadowMapTechnique();
	if (!m_ShadowMaterial->Init())
	{
		WRITE_LOG("Failed to create shadow map material", "error");
		return false;
	}

	// -- Billboard material ----
	m_BillboardMaterial = new BillboardTechnique();
	if (!m_BillboardMaterial->Init())
	{
		WRITE_LOG("Failed to create billboard material", "error");
		return false;
	}

	// -- Terrain material ----
	m_TerrainMaterial = new TerrainTechnique();
	if (!m_TerrainMaterial->Init())
	{
		WRITE_LOG("Failed to create terrain material", "error");
		return false;
	}
	m_TerrainMaterial->Use();
	for (int i = 0; i < 5; ++i)
	{
		m_TerrainMaterial->setTexSampler(i, i);
	}

	m_TerrainMaterial->setDirectionalLight(m_DirectionalLight);

	// -- Lava material ----
	m_LavaMaterial = new LavaTechnique();
	if (!m_LavaMaterial->Init())
	{
		WRITE_LOG("Failed to create lava material", "error");
		return false;
	}

	m_LavaMaterial->Use();
	m_LavaMaterial->setTexSampler(0);
	m_LavaMaterial->setResolution(Vec2(
		(float)Screen::Instance()->ScreenWidth(), (float)Screen::Instance()->ScreenHeight()));

	return true;
}

void Renderer::ReloadShaders()
{
}

Texture* Renderer::GetTexture(size_t index)
{
	return index < m_Textures.size() ?  m_Textures[index] : nullptr;
}

void Renderer::ShadowPass()
{
	if (!m_ShadowFBO)
		return;

	// Bind the Off screen frame buffer
	m_ShadowFBO->BindForWriting();

	// Clear to offscreen frameBuffer
	glClear(GL_DEPTH_BUFFER_BIT);
	m_ShadowMaterial->Use();

	// Update Light camera with spot light that it's using for shadows
	m_LightCamera->SetPosition(m_SpotLights[0].Position);
	m_LightCamera->SetDirection(m_SpotLights[0].Direction);
	m_LightCamera->Update();

	// Here you want to render the meshes that you want to check for shadows
	// We know the floor is last for this hardcoded shit so we dont want that
	
	// -1 because floor is last (hacky i know)
	for (size_t i = 0; i < m_GameObjects.size() -1; ++i)
	{
		Transform* t = m_GameObjects[i]->GetComponent<Transform>();
		MeshRenderer* mr = m_GameObjects[i]->GetComponent<MeshRenderer>();

		if (!t || !mr)
			continue;

		m_ShadowMaterial->setWvpXform(m_LightCamera->Projection() * m_LightCamera->View() * t->GetModelXform());
		m_ShadowMaterial->setTextureUnit(1);

		this->RenderMesh(mr, false);

		glBindVertexArray(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::Render()
{
	for (auto i = m_GameObjects.begin(); i != m_GameObjects.end(); ++i)
	{
		(*i)->Update();
	}

	bool useLighting = true;

	if(DO_SHADOWS)
		ShadowPass();
	
	// Clear color buffer  
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Camera->Update();

	// Render Skybox here, pass camera and mesh
	//m_SkyBoxMaterial->Render(m_Camera, m_SkyboxMesh, this);
	if (m_Camera->HasSkybox())
	{
		RenderSkybox(m_Camera);
	}

	if (!useLighting)
	{
		m_DiffuseMaterial->Use();
		m_DiffuseMaterial->setProjXform(m_Camera->Projection());
		m_DiffuseMaterial->setViewXform(m_Camera->View());
	}
	else
	{
		m_LightMaterial->Use();

		// Update spot light, to point at male model
		m_SpotLights[0].Position = Vec3(sinf(Time::ElapsedTime() * 2.0f), 5, 5.0f);
		m_SpotLights[0].Direction = Vec3(-0.5f, -1.0f, 0.0f)- m_SpotLights[0].Position;

		m_LightMaterial->setSpotLights(1, m_SpotLights);
		m_LightMaterial->setPointLights(0, nullptr);//m_PointLights);
		m_LightMaterial->setEyeWorldPos(m_Camera->Position());
	}

	int i = 0;
	
	// ---- Render Meshes ----
	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		Transform* t = m_GameObjects[i]->GetComponent<Transform>();
		MeshRenderer* mr = m_GameObjects[i]->GetComponent<MeshRenderer>();

		if (!t || !mr)
			continue;

		Mat4 xform = t->GetModelXform();

		if (!useLighting)
		{
			m_DiffuseMaterial->setModelXform(xform);
		}
		else
		{
			m_LightMaterial->setWorldMatrix(xform);
			m_LightMaterial->setWVP(m_Camera->Projection() * m_Camera->View() * xform);

			// This is the object you expect to receive shadows, in this case it's the hard coded floor
			m_LightMaterial->setLightWVP(m_LightCamera->Projection() * m_LightCamera->View() * xform);

			// Use this for sampler in light shader
			if (DO_SHADOWS)
				m_ShadowFBO->BindForReading(GL_TEXTURE1);
		}

		this->RenderMesh(mr);
	}

	// Render Lava test mesh
	m_LavaMaterial->Use();
	m_LavaMaterial->setTime(Time::ElapsedTime() * 0.16f);
	m_LavaMaterial->setWvpXform(m_Camera->Projection() * m_Camera->View() * LAVA_XFORM);

	this->RenderMesh(m_LavaTestMesh);

	// --- Render New fancy Terrain ----
	if (m_Terrain)
	{
		m_Terrain->Render(this, m_Camera, Vec4(1.0f));
	}

	// ---- Render Billboards ----
	if (m_TreeBillboardList)
	{
		m_TreeBillboardList->Render(this, m_Camera->Projection() * m_Camera->View(), m_Camera->Position(), m_Camera->Right());
	}

	// -- Render Text ----
	RenderText("CGR Engine", 8, 16);
	RenderText("Cam Fwd: " + util::vec3_to_str(m_Camera->Forward()), 8, 32);
}

void Renderer::RenderMesh(MeshRenderer* meshInstance, bool withTextures)
{
	Mesh* thisMesh = m_Meshes[meshInstance->MeshIndex];
	if (!thisMesh)
		return;

	glBindVertexArray(thisMesh->m_VAO);

	int meshIndex = 0;
	for (std::vector<SubMesh>::iterator j = thisMesh->m_SubMeshes.begin(); j != thisMesh->m_SubMeshes.end(); j++)
	{
		SubMesh subMesh = (*j);

		// Bind the textures for this mesh instance
		if (withTextures)
		{
			for (auto tex = meshInstance->m_SubMeshTextures[meshIndex].begin();
				tex != meshInstance->m_SubMeshTextures[meshIndex].end(); ++tex)
			{
				m_Textures[meshInstance->m_TextureHandles[(*tex)]]->Bind();
			}
		}

		if (subMesh.NumIndices > 0)
		{
			glDrawElementsBaseVertex(GL_TRIANGLES,
				subMesh.NumIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * subMesh.BaseIndex),
				subMesh.BaseVertex);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, subMesh.NumVertices);
		}

		++meshIndex;
	}

	glBindVertexArray(0);
}

void Renderer::RenderSkybox(BaseCamera* cam)
{
	// NEED THIS WHEN DEFERRED
	//glEnable(GL_DEPTH_TEST);
	
	// Use skybox material
	m_SkyBoxMaterial->Use();

	SkyboxSettings* sb = cam->SkyBoxParams();
	if (!sb)
		return;

	// States
	GLint oldCullMode, oldDepthFunc;
	glGetIntegerv(GL_CULL_FACE_MODE, &oldCullMode);
	glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	Mat4 model = glm::translate(IDENTITY, cam->Position())
		* glm::scale(IDENTITY, Vec3(sb->scale));

	m_SkyBoxMaterial->setWVP(cam->Projection() * cam->View() * model);
	m_SkyBoxMaterial->setTextureUnit(0);

	// Render mesh with texture here, THIS SHOULDNT BE HERE
	glBindVertexArray(m_Meshes[CUBE_MESH]->m_VAO);
	Texture* t = m_Textures[sb->textureIndex];
	if (t) t->Bind();

	for (std::vector<SubMesh>::iterator i = m_Meshes[CUBE_MESH]->m_SubMeshes.begin();
		i != m_Meshes[CUBE_MESH]->m_SubMeshes.end(); i++)
	{
		SubMesh subMesh = (*i);

		if (subMesh.NumIndices > 0)
		{
			glDrawElementsBaseVertex(GL_TRIANGLES,
				subMesh.NumIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * subMesh.BaseIndex),
				subMesh.BaseVertex);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, subMesh.NumVertices);
		}
	}

	glBindVertexArray(0);

	glCullFace(oldCullMode);
	glDepthFunc(oldDepthFunc);
	glDisable(GL_DEPTH_TEST);
}

void Renderer::RenderText(const std::string& txt, float x, float y, FontAlign fa, const Colour& colour)
{
	if (!m_Font)
		return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//if (norm)
	//{
	//	x *= 0.5f;
	//	y *= 0.5f;
	//	x += 0.5f;
	//	y += 0.5f;
	//	x *= (float)Window::FrameBufferWidth();
	//	y *= (float)Window::FrameBufferHeight();
	//}

	// Activate corresponding render state	
	m_FontMaterial->Use();

	Mat4 projection = glm::ortho(0.0f, (float)Screen::Instance()->FrameBufferWidth(), 
		0.0f, 
		(float)Screen::Instance()->FrameBufferHeight());

	
	m_FontMaterial->setProjection(projection);
	m_FontMaterial->setColour(colour.Normalize());

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(m_Font->m_Vao);

	// Iterate through all characters
	std::string::const_iterator i;
	for (i = txt.begin(); i != txt.end(); ++i)
	{
		Character ch = m_Font->m_Characters[(*i)];

		float xpos = x + ch.bearingX;
		float ypos = y - (ch.sizeY - ch.bearingY);

		float w = (float)ch.sizeX;
		float h = (float)ch.sizeY;

		if (fa == FontAlign::Centre)
		{
			xpos -= ((ch.advance >> 6)) * (txt.size() / 2);
		}
		else if (fa == FontAlign::Right)
		{
			xpos -= ((ch.advance >> 6)) * (txt.size());
		}

		// Update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_Font->m_Vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6); // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);
}

void Renderer::Close()
{
	// Clear meshes
	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		SAFE_DELETE(m_Meshes[i]);
	}
	SAFE_DELETE(m_LavaTestMesh);
	m_Meshes.clear();

	// Clear Textures
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		SAFE_DELETE(m_Textures[i]);
	}
	m_Textures.clear();

	// Clear game objects
	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		SAFE_CLOSE(m_GameObjects[i]);
	}
	m_GameObjects.clear();

	// Clean materials
	SAFE_CLOSE(m_FontMaterial);
	SAFE_CLOSE(m_LightMaterial);
	SAFE_CLOSE(m_DiffuseMaterial);
	SAFE_CLOSE(m_SkyBoxMaterial);
	SAFE_CLOSE(m_ShadowMaterial);
	SAFE_CLOSE(m_BillboardMaterial);
	SAFE_CLOSE(m_TerrainMaterial);
	SAFE_CLOSE(m_LavaMaterial);

	// Other objects
	SAFE_CLOSE(m_Font);
	SAFE_CLOSE(m_CameaObj);
	SAFE_CLOSE(m_LightCameaObj);
	SAFE_DELETE(m_ShadowFBO);
	SAFE_DELETE(m_TreeBillboardList);
	SAFE_DELETE(m_Terrain);
}

*/