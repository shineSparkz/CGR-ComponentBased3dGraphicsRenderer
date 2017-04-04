#include "ResourceManager.h"

#include "LogFile.h"
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"
#include "ShaderProgram.h"
#include "Image.h"
#include "ResId.h"
#include "Screen.h"
#include "GBuffer.h"
#include "Font.h"
#include "UniformBlockManager.h"

const ShaderAttrib POS_ATTR{ 0, "vertex_position" };
const ShaderAttrib NORM_ATTR{ 1, "vertex_normal" };
const ShaderAttrib TEX_ATTR{ 2, "vertex_texcoord" };
const ShaderAttrib TAN_ATTR{ 3, "vertex_tangent" };

void closeShaders(std::vector<Shader>& shaders)
{
	for (auto s = shaders.begin(); s != shaders.end(); ++s)
	{
		s->Close();
	}
}

bool ResourceManager::CreateDefaultResources()
{
	bool success = true;
	success &= this->loadDefaultForwardShaders();
	success &= this->loadDefaultDeferredShaders();
	success &= this->loadDefaultTextures();
	success &= this->loadDefaultMeshes();
	success &= this->loadDefaultFonts();
	return success;
}

bool ResourceManager::loadDefaultForwardShaders()
{
	// ---- Fwd lighting (Fwd) ----
	{
		//Shader lights(GL_FRAGMENT_SHADER);
		Shader geom_vs(GL_VERTEX_SHADER);
		Shader fwd_fs(GL_FRAGMENT_SHADER);

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
		//shaders.push_back(lights);

		if (!this->CreateShaderProgram(shaders, STD_FWD_LIGHTING))
			return false;
	}

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

		if (!this->CreateShaderProgram(shaders, FONT_SHADER))
		{
			return false;
		}
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

		if (!this->CreateShaderProgram(shaders, SKYBOX_SHADER))
			return false;

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

		if (!this->CreateShaderProgram(shaders, BILLBOARD_SHADER))
		{
			return false;
		}
	}

	return true;
}

bool ResourceManager::loadDefaultDeferredShaders()
{
	Vec2 screenSize((float)Screen::Instance()->FrameBufferWidth(), (float)Screen::Instance()->FrameBufferHeight());

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

		if (!this->CreateShaderProgram(shaders, TERRAIN_SHADER))
			return false;

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

		if (!this->CreateShaderProgram(shaders, LAVA_SHADER))
			return false;

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

		if (!this->CreateShaderProgram(shaders, STD_DEF_GEOM_SHADER))
			return false;

		int sampler = 0;
		m_Shaders[STD_DEF_GEOM_SHADER]->Use();
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

		if (!this->CreateShaderProgram(shaders, STD_DEF_STENCIL_SHADER))
			return false;
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

		if (!this->CreateShaderProgram(shaders, STD_DEF_PNT_LIGHT_SHADER))
			return false;

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

		if (!this->CreateShaderProgram(shaders, STD_DEF_DIR_LIGHT_SHADER))
			return false;

		int p = (int)GBuffer::TexTypes::Position;
		int d = (int)GBuffer::TexTypes::Diffuse;
		int n = (int)GBuffer::TexTypes::Normal;
		float matSpec = 0.2f;

		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->Use();

		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<int>("u_PositionMap", &p);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<int>("u_ColourMap", &d);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<int>("u_NormalMap", &n);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<Vec2>("u_ScreenSize", &screenSize);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<float>("u_MatSpecularIntensity", &matSpec);
		m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<float>("u_SpecularPower", &matSpec);
	}

	return true;
}

bool ResourceManager::loadDefaultTextures()
{
	bool success = true;
	success &= this->LoadTexture("../resources/textures/male_body_low_albedo.tga", MALE_TEX1, GL_TEXTURE0);
	success &= this->LoadTexture("../resources/textures/male_body_high_albedo.tga", MALE_TEX2, GL_TEXTURE0);
	success &= this->LoadTexture("../resources/textures/dino_diffuse.tga", DINO_TEX, GL_TEXTURE0);
	success &= this->LoadTexture("../resources/textures/terrain.tga", WALL_TEX, GL_TEXTURE0);
	success &= this->LoadTexture("../resources/textures/bricks/bricks.tga", BRICK_TEX, GL_TEXTURE0);
	success &= this->LoadTexture("../resources/textures/bricks/bricks_normal.tga", BRICK_NORM_TEX, GL_TEXTURE2);
	success &= this->LoadTexture("../resources/textures/default_normal_map.tga", FAKE_NORMAL_TEX, GL_TEXTURE2);
	success &= this->LoadTexture("../resources/textures/billboards/grass.tga", TREE_BILLBOARD_TEX, GL_TEXTURE0);

	success &= this->LoadTexture("../resources/textures/terrain/fungus.tga", TERRAIN1_TEX, GL_TEXTURE0);
	success &= this->LoadTexture("../resources/textures/terrain/sand_grass.tga", TERRAIN2_TEX, GL_TEXTURE1);
	success &= this->LoadTexture("../resources/textures/terrain/rock.tga", TERRAIN3_TEX, GL_TEXTURE2);
	success &= this->LoadTexture("../resources/textures/terrain/sand.tga", TERRAIN4_TEX, GL_TEXTURE3);
	success &= this->LoadTexture("../resources/textures/terrain/path.tga", TERRAIN5_TEX, GL_TEXTURE4);
	success &= this->LoadTexture("../resources/textures/noise.tga", LAVA_NOISE_TEX, GL_TEXTURE0);

	std::string s[6] =
	{
		"../resources/textures/skybox/rightr.tga",
		"../resources/textures/skybox/leftr.tga",
		"../resources/textures/skybox/topr.tga",
		"../resources/textures/skybox/bottomr.tga",
		"../resources/textures/skybox/backr.tga",
		"../resources/textures/skybox/frontr.tga"
	};

	success &= this->LoadCubeMap(s, SKYBOX_TEX, GL_TEXTURE0);

	return success;
}

bool ResourceManager::loadDefaultMeshes()
{
	bool success = true;
	success &= LoadMesh("cube.obj", CUBE_MESH, false, false);
	success &= LoadMesh("quad.obj", QUAD_MESH, false, false);
	success &= LoadMesh("sphere.obj", SPHERE_MESH, false, false);
	success &= LoadMesh("male.obj", MALE_MESH, false, false);
	success &= LoadMesh("dino.obj", DINO_MESH, false, false);
	return success;
}

bool ResourceManager::loadDefaultFonts()
{
	bool success = true;
	success &= this->LoadFont("../resources/fonts/cour.ttf", FONT_COUR, 24);
	return success;
}

bool ResourceManager::LoadFont(const std::string& path, size_t key, int size)
{
	if (m_Fonts.find(key) != m_Fonts.end())
	{
		WRITE_LOG("Font already exists", "error");
		return false;
	}

	Font* m_Font = new Font();
	m_Fonts[key] = m_Font;
	if (!m_Font->CreateFont(path, size))
	{
		WRITE_LOG("FONT LOAD FAIL", "error");
		return false;
	}

	return true;
}

bool ResourceManager::LoadMesh(const std::string& path, size_t key_store, bool tangents, bool withTextures)
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

bool ResourceManager::LoadTexture(const std::string& path, size_t key_store, int glTextureIndex)
{
	Image i;
	std::string full_path = path;
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

bool ResourceManager::LoadCubeMap(std::string path[6], size_t key_store, int glTextureIndex)
{
	// Load Images for skybox
	Image* images[6];
	Image i0;
	Image i1;
	Image i2;
	Image i3;
	Image i4;
	Image i5;

	if (!i0.LoadImg(path[0].c_str()))
		return false;
	if (!i1.LoadImg(path[1].c_str()))
		return false;
	if (!i2.LoadImg(path[2].c_str()))
		return false;
	if (!i3.LoadImg(path[3].c_str()))
		return false;
	if (!i4.LoadImg(path[4].c_str()))
		return false;
	if (!i5.LoadImg(path[5].c_str()))
		return false;

	images[0] = &i0;
	images[1] = &i1;
	images[2] = &i2;
	images[3] = &i3;
	images[4] = &i4;
	images[5] = &i5;

	Texture* cubeMapTex = new Texture("cubemap" + std::to_string(m_Textures.size()), GL_TEXTURE_CUBE_MAP, GL_TEXTURE0);
	m_Textures[key_store] = cubeMapTex;
	return cubeMapTex->Create(images);
}

bool ResourceManager::CreateShaderProgram(std::vector<Shader>& shaders, size_t key)
{
	if (m_Shaders.find(key) != m_Shaders.end())
	{
		closeShaders(shaders);
		WRITE_LOG("Error: This shader resource already exists: " + std::to_string(key), "error");
		return false;
	}

	ShaderProgram* sp = new ShaderProgram();
	m_Shaders[key] = sp;

	bool success = true;

	if (!sp->CreateProgram(shaders, "frag_colour", 0))
	{
		WRITE_LOG("Failed to create shader program", "error");
		success = false;
	}

	closeShaders(shaders);
	return success;
}

size_t ResourceManager::GetNumSubMeshesInMesh(size_t meshIndex) const
{
	auto r = m_Meshes.find(meshIndex);
	if (r != m_Meshes.end())
	{
		return r->second->GetNumSubMeshes();
	}
	else
	{
		WRITE_LOG("error invalid mesh", "error");
		return 0;
	}
}

size_t ResourceManager::GetNumTextures() const
{
	return m_Textures.size();
}

Texture* ResourceManager::GetTexture(size_t index)
{
	auto r = m_Textures.find(index);
	if (r != m_Textures.end())
	{
		return r->second;
	}
	else
	{
		WRITE_LOG("error invalid texture", "error");
		return nullptr;
	}
}

ShaderProgram* ResourceManager::GetShader(size_t index)
{
	auto r = m_Shaders.find(index);
	if (r != m_Shaders.end())
	{
		return r->second;
	}
	else
	{
		WRITE_LOG("error invalid shader", "error");
		return nullptr;
	}
}

void ResourceManager::Close()
{
	// Clear meshes
	for (auto i = m_Meshes.begin(); i != m_Meshes.end(); ++i)
	{
		SAFE_DELETE(i->second);
	}
	m_Meshes.clear();

	// Clean shaders
	for (auto i = m_Shaders.begin(); i != m_Shaders.end(); ++i)
	{
		SAFE_CLOSE(i->second);
	}
	m_Shaders.clear();

	// Clear Textures
	for (auto i = m_Textures.begin(); i != m_Textures.end(); ++i)
	{
		SAFE_DELETE(i->second);
	}
	m_Textures.clear();

	for (auto i = m_Fonts.begin(); i != m_Fonts.end(); ++i)
	{
		SAFE_CLOSE(i->second);
	}
	m_Fonts.clear();
}

