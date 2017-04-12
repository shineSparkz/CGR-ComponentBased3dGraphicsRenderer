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
#include "Material.h"

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

		if (!geom_vs.LoadShader("../resources/shaders/new_lights/forward_geom_vs.glsl"))
		{
			WRITE_LOG("Geom shader failed compile", "error");
			return false;
		}

		if (!fwd_fs.LoadShader("../resources/shaders/new_lights/forward_lights_fs.glsl"))
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

		if (!this->CreateShaderProgram(shaders, SHADER_LIGHTING_FWD))
			return false;
	}

	// ---- Font material (Fwd) ----
	{
		Shader font_vert(GL_VERTEX_SHADER);
		Shader font_frag(GL_FRAGMENT_SHADER);
		if (!font_vert.LoadShader("../resources/shaders/font/font_vs.glsl")) { return false; }
		if (!font_frag.LoadShader("../resources/shaders/font/font_fs.glsl")) { return false; }
		font_vert.AddAttribute(POS_ATTR);
		
		std::vector<Shader> shaders;
		shaders.push_back(font_vert);
		shaders.push_back(font_frag);

		if (!this->CreateShaderProgram(shaders, SHADER_FONT_FWD))
		{
			return false;
		}
	}

	// ---- Skybox (Fwd) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/skybox/skybox_vs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/skybox/skybox_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);
		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		if (!this->CreateShaderProgram(shaders, SHADER_SKYBOX_ANY))
			return false;

		// One off values
		int texUnit = 0;
		m_Shaders[SHADER_SKYBOX_ANY]->Use();
		m_Shaders[SHADER_SKYBOX_ANY]->SetUniformValue<int>("cube_sampler", &texUnit);
	}

	// ---- Bill board (Fwd) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader geom(GL_GEOMETRY_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/billboard/billboard_vs.glsl")) { return false; }
		if (!geom.LoadShader("../resources/shaders/billboard/billboard_gs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/billboard/billboard_forward_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(geom);
		shaders.push_back(frag);

		if (!this->CreateShaderProgram(shaders, SHADER_BILLBOARD_FWD))
		{
			return false;
		}
	}

	// ---- Normal Display (Fwd) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader geom(GL_GEOMETRY_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/normal_debug/normal_display_vs.glsl")) { return false; }
		if (!geom.LoadShader("../resources/shaders/normal_debug/normal_display_gs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/normal_debug/normal_display_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);
		vert.AddAttribute(NORM_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(geom);
		shaders.push_back(frag);

		if (!this->CreateShaderProgram(shaders, SHADER_NORMAL_DISP_FWD))
		{
			return false;
		}
	}

	// ---- Terrain (Fwd) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/new_lights/forward_geom_vs.glsl")) { return false; }
		//if (!vert.LoadShader("../resources/shaders/terrain/terrain_vs.glsl")) { return false; }

		if (!frag.LoadShader("../resources/shaders/terrain/terrain_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);
		vert.AddAttribute(NORM_ATTR);
		vert.AddAttribute(TEX_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		if (!this->CreateShaderProgram(shaders, SHADER_TERRAIN_DEF))
			return false;

		m_Shaders[SHADER_TERRAIN_DEF]->Use();
		for (int i = 0; i < 5; ++i)
		{
			m_Shaders[SHADER_TERRAIN_DEF]->SetUniformValue<int>("u_Sampler" + std::to_string(i), &i);
		}
	}

	// ---- Frustum (Fwd) ----
	{
		Shader vert(GL_VERTEX_SHADER);
		Shader frag(GL_FRAGMENT_SHADER);
		if (!vert.LoadShader("../resources/shaders/frustum/frustum.vs.glsl")) { return false; }
		if (!frag.LoadShader("../resources/shaders/frustum/frustum_fs.glsl")) { return false; }
		vert.AddAttribute(POS_ATTR);

		std::vector<Shader> shaders;
		shaders.push_back(vert);
		shaders.push_back(frag);

		if (!this->CreateShaderProgram(shaders, SHADER_FRUSTUM))
			return false;
	}

	return true;
}

bool ResourceManager::loadDefaultDeferredShaders()
{
	Vec2 screenSize((float)Screen::Instance()->FrameBufferWidth(), (float)Screen::Instance()->FrameBufferHeight());

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

		if (!this->CreateShaderProgram(shaders, SHADER_LAVA_FWD))
			return false;

		m_Shaders[SHADER_LAVA_FWD]->Use();
		int sampler = 0;
		m_Shaders[SHADER_LAVA_FWD]->SetUniformValue<int>("u_Sampler", &sampler);
		m_Shaders[SHADER_LAVA_FWD]->SetUniformValue<Vec2>("u_Resolution", &screenSize);
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

		if (!this->CreateShaderProgram(shaders, SHADER_GEOM_PASS_DEF))
			return false;

		int sampler = 0;
		m_Shaders[SHADER_GEOM_PASS_DEF]->Use();
		m_Shaders[SHADER_GEOM_PASS_DEF]->SetUniformValue<int>("u_ColourMap", &sampler);
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

		if (!this->CreateShaderProgram(shaders, SHADER_STENCIL_PASS_DEF))
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

		if (!this->CreateShaderProgram(shaders, SHADER_POINT_LIGHT_PASS_DEF))
			return false;

		int p = (int)GBuffer::TexTypes::Position;
		int d = (int)GBuffer::TexTypes::Diffuse;
		int n = (int)GBuffer::TexTypes::Normal;
		float matSpec = 0.1f;

		m_Shaders[SHADER_POINT_LIGHT_PASS_DEF]->Use();
		m_Shaders[SHADER_POINT_LIGHT_PASS_DEF]->SetUniformValue<int>("u_PositionMap", &p);
		m_Shaders[SHADER_POINT_LIGHT_PASS_DEF]->SetUniformValue<int>("u_ColourMap", &d);
		m_Shaders[SHADER_POINT_LIGHT_PASS_DEF]->SetUniformValue<int>("u_NormalMap", &n);
		m_Shaders[SHADER_POINT_LIGHT_PASS_DEF]->SetUniformValue<Vec2>("u_ScreenSize", &screenSize);
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

		if (!this->CreateShaderProgram(shaders, SHADER_DIR_LIGHT_PASS_DEF))
			return false;

		int p = (int)GBuffer::TexTypes::Position;
		int d = (int)GBuffer::TexTypes::Diffuse;
		int n = (int)GBuffer::TexTypes::Normal;
		float matSpec = 0.2f;

		m_Shaders[SHADER_DIR_LIGHT_PASS_DEF]->Use();

		m_Shaders[SHADER_DIR_LIGHT_PASS_DEF]->SetUniformValue<int>("u_PositionMap", &p);
		m_Shaders[SHADER_DIR_LIGHT_PASS_DEF]->SetUniformValue<int>("u_ColourMap", &d);
		m_Shaders[SHADER_DIR_LIGHT_PASS_DEF]->SetUniformValue<int>("u_NormalMap", &n);
		m_Shaders[SHADER_DIR_LIGHT_PASS_DEF]->SetUniformValue<Vec2>("u_ScreenSize", &screenSize);
	}

	return true;
}

void ResourceManager::AddMaterialSet(size_t key, const std::map<unsigned, Material*> materials)
{
	if (m_Materials.find(key) != m_Materials.end())
	{
		return;
	}
	else
	{
		m_Materials[key] = materials;
	}
}

bool ResourceManager::MaterialSetExists(size_t key) const
{
	if (m_Materials.find(key) != m_Materials.end())
	{
		return true;
	}

	return false;
}

Texture* ResourceManager::LoadTexture(const std::string& textureFile, int textureSampler)
{
	Image i;
	if (!i.LoadImg(textureFile.c_str()))
	{
		WRITE_LOG("Failed to load texture: " + textureFile, "error");
		return nullptr;
	}

	Texture* t = new Texture(textureFile, GL_TEXTURE_2D, textureSampler);

	if (!t->Create(&i))
	{
		WRITE_LOG("Failed to create texture: " + textureFile, "error");
		return nullptr;
	}

	return t;
}

bool ResourceManager::loadDefaultTextures()
{
	bool success = true;

	// Load Male mesh material set
	{
		Texture* male_low_diff = this->LoadTexture("../resources/textures/male_body_low_albedo.tga", GL_TEXTURE0);
		Texture* male_high_diff = this->LoadTexture("../resources/textures/male_body_high_albedo.tga", GL_TEXTURE0);
		if (male_low_diff && male_high_diff)
		{
			m_Materials[MATERIALS_MALE][0] = new Material();
			m_Materials[MATERIALS_MALE][1] = new Material();

			m_Materials[MATERIALS_MALE][0]->diffuse_map = male_high_diff;
			m_Materials[MATERIALS_MALE][1]->diffuse_map = male_low_diff;
		}
	}

	// Load Grass material set
	{
		Texture* grass_diff = this->LoadTexture("../resources/textures/terrain.tga", GL_TEXTURE0);
		if (grass_diff)
		{
			m_Materials[MATERIALS_GRASS][0] = new Material();

			m_Materials[MATERIALS_GRASS][0]->diffuse_map = grass_diff;
		}
	}

	// Load Brick material set
	{
		Texture* brick_diff = this->LoadTexture("../resources/textures/bricks/bricks.tga", GL_TEXTURE0);
		Texture* brick_norm = this->LoadTexture("../resources/textures/bricks/bricks_normal.tga", GL_TEXTURE2);

		if (brick_diff && brick_norm)
		{
			m_Materials[MATERIALS_BRICKS][0] = new Material();
			m_Materials[MATERIALS_BRICKS][0]->diffuse_map = brick_diff;
			m_Materials[MATERIALS_BRICKS][0]->normal_map = brick_norm;
		}
	}

	// Load Terrain material set
	{
		Texture* low =			this->LoadTexture("../resources/textures/terrain/fungus.tga",		GL_TEXTURE0);
		Texture* med =			this->LoadTexture("../resources/textures/terrain/sand_grass.tga",	GL_TEXTURE1);
		Texture* high =			this->LoadTexture("../resources/textures/terrain/rock.tga",			GL_TEXTURE2);
		Texture* path =			this->LoadTexture("../resources/textures/terrain/sand.tga",			GL_TEXTURE3);
		Texture* path_samp =	this->LoadTexture("../resources/textures/terrain/path.tga",			GL_TEXTURE4);

		if (low && med && high && path && path_samp)
		{
			m_Materials[MATERIALS_TERRAIN][0] = new Material();
			m_Materials[MATERIALS_TERRAIN][0]->diffuse_map = low;

			m_Materials[MATERIALS_TERRAIN][1] = new Material();
			m_Materials[MATERIALS_TERRAIN][1]->diffuse_map = med;

			m_Materials[MATERIALS_TERRAIN][2] = new Material();
			m_Materials[MATERIALS_TERRAIN][2]->diffuse_map = high;

			m_Materials[MATERIALS_TERRAIN][3] = new Material();
			m_Materials[MATERIALS_TERRAIN][3]->diffuse_map = path;

			m_Materials[MATERIALS_TERRAIN][4] = new Material();
			m_Materials[MATERIALS_TERRAIN][4]->diffuse_map = path_samp;
		}
	}

	// From Loaded Scene
	success &= this->LoadTexture("../resources/textures/billboards/grass.tga", TEX_GRASS_BILLBOARD, GL_TEXTURE0);
	success &= this->LoadTexture("../resources/textures/noise.tga", TEX_NOISE, GL_TEXTURE0);

	std::string s[6] =
	{
		"../resources/textures/skybox/rightr.tga",
		"../resources/textures/skybox/leftr.tga",
		"../resources/textures/skybox/topr.tga",
		"../resources/textures/skybox/bottomr.tga",
		"../resources/textures/skybox/backr.tga",
		"../resources/textures/skybox/frontr.tga"
	};

	success &= this->LoadCubeMap(s, TEX_SKYBOX_DEFAULT, GL_TEXTURE0);

	return success;
}

bool ResourceManager::loadDefaultMeshes()
{
	bool success = true;
	success &= LoadMesh("cube.obj", MESH_ID_CUBE, true, false, 0);
	success &= LoadMesh("quad.obj", MESH_ID_QUAD, false, false, 0);
	success &= LoadMesh("sphere.obj", MESH_ID_SPHERE, false, false, 0);
	success &= LoadMesh("male.obj", MESH_ID_MALE, false, false, 0);
	return success;
}

bool ResourceManager::loadDefaultFonts()
{
	bool success = true;
	success &= this->LoadFont("../resources/fonts/cour.ttf", FONT_COURIER, 24);
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

bool ResourceManager::LoadMesh(const std::string& path, size_t key_store, bool tangents, bool withTextures, unsigned materialSet)
{
	if (m_Meshes.find(key_store) != m_Meshes.end())
	{
		WRITE_LOG("Tried to use same mesh key twice", "error");
		return false;
	}

	Mesh* mesh = new Mesh();
	m_Meshes[key_store] = mesh;
	if (!mesh->Load(path, tangents, withTextures, materialSet, this))
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

bool ResourceManager::CheckMeshExists(size_t key)
{
	if (m_Meshes.find(key) != m_Meshes.end())
	{
		return true;
	}

	return false;
}

bool ResourceManager::CheckTextureExists(size_t key)
{
	if (m_Textures.find(key) != m_Textures.end())
	{
		return true;
	}

	return false;
}

bool ResourceManager::CheckShaderExists(size_t key)
{
	if (m_Shaders.find(key) != m_Shaders.end())
	{
		return true;
	}

	return false;
}

bool ResourceManager::CheckFontExists(size_t key)
{
	if (m_Fonts.find(key) != m_Fonts.end())
	{
		return true;
	}

	return false;
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

	// Clean Materials
	for (auto i = m_Materials.begin(); i != m_Materials.end(); ++i)
	{
		for (auto j = i->second.begin(); j != i->second.end(); ++j)
		{
			j->second->Clean();
			SAFE_DELETE(j->second);
		}
	}

	m_Materials.clear();

	// Clean Fonts
	for (auto i = m_Fonts.begin(); i != m_Fonts.end(); ++i)
	{
		SAFE_CLOSE(i->second);
	}
	m_Fonts.clear();
}

Mesh* ResourceManager::GetMesh(size_t index) const
{
	auto m = m_Meshes.find(index);
	if (m != m_Meshes.end())
	{
		return m->second;
	}
	else
	{
		WRITE_LOG("error invalid texture", "error");
		return nullptr;
	}
}