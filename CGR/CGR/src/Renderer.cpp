#include "Renderer.h"

#include "OpenGlLayer.h"
#include "Vertex.h"
#include "LogFile.h"
#include <sstream>
#include "utils.h"

// Remove
#include "Time.h"
#include "TextFile.h"
#include "Image.h"
#include "Screen.h"
#include "math_utils.h"
#include "Camera.h"

#include "Technique.h"
#include "Mesh.h"
#include "Texture.h"
#include "Font.h"
#include "Screen.h"
#include "ShadowFrameBuffer.h"
#include "BillboardList.h"
#include "Terrain.h"
#include "GBuffer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

float CalcPointLightBSphere(const PointLight& Light)
{
	float MaxChannel = fmax(fmax(Light.Color.x, Light.Color.y), Light.Color.z);

	float ret = (-Light.Attenuation.Linear + sqrtf(Light.Attenuation.Linear * Light.Attenuation.Linear -
		4 * Light.Attenuation.Exp * (Light.Attenuation.Exp - 256 * MaxChannel * Light.DiffuseIntensity)))
		/
		(2 * Light.Attenuation.Exp);
	return ret;
}

void Renderer::RenderMesh(Mesh* thisMesh)
{
	glBindVertexArray(thisMesh->m_VAO);

	for (std::vector<SubMesh>::iterator j = thisMesh->m_MeshLayouts.begin(); j != thisMesh->m_MeshLayouts.end(); j++)
	{
		SubMesh subMesh = (*j);

		for (auto tex = subMesh.TextureIndices.begin(); tex != subMesh.TextureIndices.end(); ++tex)
		{
			m_Textures[thisMesh->m_TextureHandles[(*tex)]]->Bind();
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
	}

	glBindVertexArray(0);
}

const bool DO_SHADOWS = true;

// Transforms -- for now
const Mat4 MALE_XFORM =  glm::translate(Mat4(1.0f), Vec3(-0.5f, -1.5f, 0.0f))  *  glm::scale(Mat4(1.0f), Vec3(1.0f));
const Mat4 DINO_XFORM =  glm::translate(Mat4(1.0f), Vec3(1.8f, -1.5f, 0.0f))   *  glm::scale(Mat4(1.0f), Vec3(0.05f));
//const Mat4 CUBE1_XFORM = glm::translate(Mat4(1.0f), Vec3(-3.1f, 0.0f, 0.0f))   *  glm::scale(Mat4(1.0f), Vec3(0.5f));
const Mat4 CUBE1_XFORM = glm::translate(Mat4(1.0f), Vec3(0.0f, 1.0f, 0.0f))   *  glm::scale(Mat4(1.0f), Vec3(1.0f));

const Mat4 CUBE2_XFORM = glm::translate(Mat4(1.0f), Vec3(-2.0f, 0.0f, 0.0f))   *  glm::scale(Mat4(1.0f), Vec3(0.5f));
const Mat4 FLOOR_XFORM = glm::translate(Mat4(1.0f), Vec3(0.0f, -2.0f, -3.0f))  *  glm::scale(Mat4(1.0f), Vec3(30.0f, 0.5f, 30.0f));

const Mat4 LAVA_XFORM = glm::translate(Mat4(1.0f),  Vec3(0.0f, 4.0f, 0.0f)) *  glm::scale(Mat4(1.0f), Vec3(400.0f, 2.0f, 400.0f));

// Texture ID's -- for now
const size_t MALE_TEX1 = 0;
const size_t MALE_TEX2 = 1;
const size_t DINO_TEX = 2;
const size_t WALL_TEX = 3;
const size_t SKYBOX_TEX = 4;
const size_t BRICK_TEX = 5;
const size_t BRICK_NORM_TEX = 6;
const size_t FAKE_NORMAL_TEX = 7;
const size_t TREE_BILLBOARD_TEX = 8;
const size_t TERRAIN1_TEX = 9;//-->
const size_t TERRAIN2_TEX = 10;
const size_t TERRAIN3_TEX = 11;
const size_t TERRAIN4_TEX = 12;
const size_t TERRAIN5_TEX = 13;//--<
const size_t LAVA_NOISE_TEX = 14;

std::vector<Mat4> TRANSFORMS;

// Deferred Renderer
Renderer::Renderer() :
	m_Textures(),
	m_Font(nullptr),
	m_Camera(nullptr),
	m_Gbuffer(nullptr),

	m_FontMaterial(nullptr),
	m_GeomPassMaterial(nullptr),
	m_PointLightPassMaterial(nullptr),
	m_DirLightPassMaterial(nullptr),
	m_NullTech(nullptr)
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
	m_Camera->aspect = static_cast<float>(w / h);

	if (m_Gbuffer)
	{
		m_Gbuffer->Init();
	}
}

bool Renderer::setLights()
{
	// Dir Light
	m_DirectionalLight.Color = Vec3(1.0f, 1.0f, 1.0f);
	m_DirectionalLight.AmbientIntensity = 0.05f;
	m_DirectionalLight.DiffuseIntensity = 0.05f;
	m_DirectionalLight.Direction = Vec3(1.0f, -1.0f, 0.0f);

	// Point Lights
	m_PointLights[0].Position = Vec3(-15.0f, 2.0f, 10.0f);
	m_PointLights[1].Position = Vec3(-5.0f, 2.0f, 10.0f);
	m_PointLights[2].Position = Vec3(5.0f, 2.0f, 10.0f);
	m_PointLights[3].Position = Vec3(15.0f, 2.0f, 10.0f);

	m_PointLights[4].Position = Vec3(-15.0f, 5.0f, -10.0f);
	m_PointLights[5].Position = Vec3(-5.0f, 5.0f, -10.0f);
	m_PointLights[6].Position = Vec3(5.0f, 3.0f, -10.0f);
	m_PointLights[7].Position = Vec3(15.0f, 2.0f, -10.0f);



	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_PointLights); ++i)
	{
		//m_PointLights[i].DiffuseIntensity = random::RandFloat(0.3f, 0.9f);
		m_PointLights[i].DiffuseIntensity = 0.9f;// random::RandFloat(0.3f, 0.9f);

		//m_PointLights[i].Color = Vec3(random::RandFloat(0.1f, 1.0f), random::RandFloat(0.1f, 1.0f), random::RandFloat(0.1f, 1.0f));
		m_PointLights[i].Color = Vec3(1, 1, 1);// random::RandFloat(0.1f, 1.0f), random::RandFloat(0.1f, 1.0f), random::RandFloat(0.1f, 1.0f));

		m_PointLights[i].Attenuation.Constant = 0.0f;
		m_PointLights[i].Attenuation.Linear = 0.0f;
		m_PointLights[i].Attenuation.Exp = 0.3f;
	}

	return true;
}

bool Renderer::setCamera()
{
	// Camera
	m_Camera = new Camera();
	m_Camera->position = Vec3(0, 1.0f, 4.0f);
	m_Camera->up = Vec3(0.0f, 1.0f, 0.0f);
	m_Camera->right = Vec3(1.0f, 0.0f, 0.0f);
	m_Camera->forward = Vec3(0.0f, 0.0f, -1.0f);
	m_Camera->fov = 45.0f;
	m_Camera->aspect = static_cast<float>(Screen::Instance()->ScreenWidth() / Screen::Instance()->ScreenHeight());
	m_Camera->near = 0.1f;
	m_Camera->far = 500.0f;

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
	m_CubeMesh = new Mesh();
	if (!m_CubeMesh->Load("cube.obj", false)) { return false; }
	if (!m_CubeMesh->AddTexture(BRICK_TEX)) return false;

	m_QuadMesh = new Mesh();
	if (!m_QuadMesh->Load("quad.obj", false)) return false;

	m_SphereMesh = new Mesh();
	if (!m_SphereMesh->Load("sphere.obj", false)) return false;


	TRANSFORMS.push_back(MALE_XFORM);
	TRANSFORMS.push_back(DINO_XFORM);
	TRANSFORMS.push_back(CUBE1_XFORM);
	TRANSFORMS.push_back(CUBE2_XFORM);
	TRANSFORMS.push_back(FLOOR_XFORM);

	return true;
}

bool Renderer::createMaterials()
{
	// ---- Font material -----
	m_FontMaterial = new FontTechnique();
	if (!m_FontMaterial->Init())
	{
		WRITE_LOG("Failed to create font mat", "error");
		return false;
	}

	m_GeomPassMaterial = new GeometryPassTechnique();
	if (!m_GeomPassMaterial->Init())
	{
		WRITE_LOG("Failed to create geom pass mat", "error");
		return false;
	}
	m_GeomPassMaterial->Use();
	m_GeomPassMaterial->setColourSampler(0);

	// TODO : cjhange this on window size callback
	Vec2 screenSize((float)Screen::Instance()->FrameBufferWidth(), (float)Screen::Instance()->FrameBufferHeight());

	m_PointLightPassMaterial = new DSPointLightPassTech();
	if (!m_PointLightPassMaterial->Init())
	{
		WRITE_LOG("Failed to create point light pass mat", "error");
		return false;
	}
	m_PointLightPassMaterial->Use();
	m_PointLightPassMaterial->setColorTextureUnit(GBuffer::TexTypes::Diffuse);
	m_PointLightPassMaterial->setNormalTextureUnit(GBuffer::TexTypes::Normal);
	m_PointLightPassMaterial->setPositionTextureUnit(GBuffer::TexTypes::Position);
	m_PointLightPassMaterial->setScreenSize(screenSize);
	m_PointLightPassMaterial->setMatSpecularIntensity(0.2f);
	m_PointLightPassMaterial->setMatSpecularPower(0.2f);


	m_DirLightPassMaterial = new DSDirLightPassTech();
	if (!m_DirLightPassMaterial->Init())
	{
		WRITE_LOG("Failed to create dir light pass mat", "error");
		return false;
	}
	m_DirLightPassMaterial->Use();
	m_DirLightPassMaterial->setColorTextureUnit(GBuffer::TexTypes::Diffuse);
	m_DirLightPassMaterial->setNormalTextureUnit(GBuffer::TexTypes::Normal);
	m_DirLightPassMaterial->setPositionTextureUnit(GBuffer::TexTypes::Position);
	m_DirLightPassMaterial->setScreenSize(screenSize);
	m_DirLightPassMaterial->setDirectionalLight(m_DirectionalLight);


	m_NullTech = new NullTechnique();
	if (!m_NullTech->Init())
	{
		WRITE_LOG("Failed to create null tech mat", "error");
		return false;
	}
	return true;
}

void Renderer::ReloadShaders()
{
	/*
	GLuint vertShader, fragShader;

	if (!load_shader("../resources/shaders/test_vs.glsl", vertShader, GL_VERTEX_SHADER))
	{
	OpenGLLayer::clean_GL_shader(&vertShader);
	return;
	}

	if (!load_shader("../resources/shaders/test_fs.glsl", fragShader, GL_FRAGMENT_SHADER))
	{
	OpenGLLayer::clean_GL_shader(&vertShader);
	OpenGLLayer::clean_GL_shader(&fragShader);
	return;
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);

	// **** Dynamic ****
	glBindAttribLocation(shaderProgram, 0, "vertex_position");
	glBindAttribLocation(shaderProgram, 1, "vertex_normal");
	glBindAttribLocation(shaderProgram, 2, "vertex_texcoord");

	glBindFragDataLocation(shaderProgram, 0, "fragment_colour");

	glLinkProgram(shaderProgram);

	GLint link_status = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &link_status);

	if (link_status != GL_TRUE)
	{
	const int string_length = 1024;
	GLchar log[string_length] = "";
	glGetProgramInfoLog(shaderProgram, string_length, NULL, log);
	std::stringstream logger;
	logger << log << std::endl;
	WRITE_LOG(logger.str(), "error");
	OpenGLLayer::clean_GL_shader(&vertShader);
	OpenGLLayer::clean_GL_shader(&fragShader);
	}

	OpenGLLayer::clean_GL_shader(&vertShader);
	OpenGLLayer::clean_GL_shader(&fragShader);

	// Uniforms

	uID_modelXform = glGetUniformLocation(shaderProgram, "model_xform");
	if (uID_modelXform == INVALID_UNIFORM_LOCATION)
	{
	WRITE_LOG("Warning: Invalid uniform location, model", "warning");
	return;
	}

	uID_viewXform = glGetUniformLocation(shaderProgram, "view_xform");
	if (uID_viewXform == INVALID_UNIFORM_LOCATION)
	{
	WRITE_LOG("Warning: Invalid uniform location, view", "warning");
	return;
	}

	uID_projXform = glGetUniformLocation(shaderProgram, "proj_xform");
	if (uID_projXform == INVALID_UNIFORM_LOCATION)
	{
	WRITE_LOG("Warning: Invalid uniform location, proj", "warning");
	return;
	}

	uID_texSampler = glGetUniformLocation(shaderProgram, "texture_sampler");
	if (uID_texSampler == INVALID_UNIFORM_LOCATION)
	{
	WRITE_LOG("Warning: Invalid uniform location, tex sampler", "warning");
	//return false;
	}

	// This would be somewhere else
	glUseProgram(shaderProgram);
	*/
}

Texture* Renderer::GetTexture(size_t index)
{
	return index < m_Textures.size() ? m_Textures[index] : nullptr;
}

void Renderer::Render()
{
	// Clear color buffer  
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Camera->Update();

	m_Gbuffer->StartFrame();
	
	GeomPass();

	glEnable(GL_STENCIL_TEST);

	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_PointLights); ++i)
	{
		StencilPass(i);
		PointLightPass(i);
	}
	
	glDisable(GL_STENCIL_TEST);

	DirLightPass();
	FinalPass();

	// -- Render Text ----
	RenderText("Cam Pos: " + util::vec3_to_str(m_Camera->position), 8, 16);
	RenderText("Cam Fwd: " + util::vec3_to_str(m_Camera->forward), 8, 32);
	RenderText("Cam Up: "  + util::vec3_to_str(m_Camera->up), 8, 48);

}

void Renderer::GeomPass()
{
	m_GeomPassMaterial->Use();

	m_Gbuffer->BindForGeomPass();

	// Only geom pass updates depth buffer
	glDepthMask(GL_TRUE);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);

	for (int i = 0; i < TRANSFORMS.size(); ++i)
	{
		m_GeomPassMaterial->setWVP(m_Camera->projection * m_Camera->view * TRANSFORMS[i]);
		m_GeomPassMaterial->setWorld(TRANSFORMS[i]);

		// Render Mesh here
		this->RenderMesh(m_CubeMesh);
	}

	glDepthMask(GL_FALSE);
	//glDisable(GL_DEPTH_TEST);
}

void Renderer::StencilPass(int lightIndex)
{
	m_NullTech->Use();

	// Disable color/depth write and enable stencil
	m_Gbuffer->BindForStencilPass();
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);

	glClear(GL_STENCIL_BUFFER_BIT);

	// We need the stencil test to be enabled but we want it
	// to succeed always. Only the depth test matters.
	glStencilFunc(GL_ALWAYS, 0, 0);

	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	Mat4 LIGHT_TRANS = glm::translate(Mat4(1.0f), m_PointLights[lightIndex].Position) * glm::scale(Mat4(1.0f), Vec3(CalcPointLightBSphere(m_PointLights[lightIndex])));
	m_PointLightPassMaterial->setWVP(m_Camera->projection * m_Camera->view * LIGHT_TRANS);
	
	m_NullTech->setWVP(m_Camera->projection * m_Camera->view * LIGHT_TRANS);
	this->RenderMesh(m_SphereMesh);
}

void Renderer::PointLightPass(int lightIndex)
{
	m_Gbuffer->BindForLightPass();
	m_PointLightPassMaterial->Use();
	m_PointLightPassMaterial->setEyeWorldPos(m_Camera->position);

	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	m_PointLightPassMaterial->setPointLight(m_PointLights[lightIndex]);
	Mat4 LIGHT_TRANS = glm::translate(Mat4(1.0f), m_PointLights[lightIndex].Position) * glm::scale(Mat4(1.0f), Vec3((CalcPointLightBSphere(m_PointLights[lightIndex]))));
	m_PointLightPassMaterial->setWVP(m_Camera->projection * m_Camera->view * LIGHT_TRANS);
	this->RenderMesh(m_SphereMesh);

	glCullFace(GL_BACK);

	glDisable(GL_BLEND);
}

void Renderer::DirLightPass()
{
	glDisable(GL_CULL_FACE);
	m_Gbuffer->BindForLightPass();
	m_DirLightPassMaterial->Use();
	m_DirLightPassMaterial->setEyeWorldPos(m_Camera->position);
	
	// Should only set once
	m_DirLightPassMaterial->setWVP(Mat4(1.0f));
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	this->RenderMesh(m_QuadMesh);

	glDisable(GL_BLEND);
}

void Renderer::FinalPass()
{
	int width  = Screen::Instance()->FrameBufferWidth();
	int height = Screen::Instance()->FrameBufferHeight();

	m_Gbuffer->BindForFinalPass();
	glBlitFramebuffer(0, 0, width, height,
		0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
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
	/*
	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		SAFE_DELETE(m_Meshes[i]);
	}
	m_Meshes.clear();
	*/

	SAFE_DELETE(m_CubeMesh);
	SAFE_DELETE(m_QuadMesh);
	SAFE_DELETE(m_SphereMesh);


	// Clear Textures
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		SAFE_DELETE(m_Textures[i]);
	}
	m_Textures.clear();

	// Clean materials
	SAFE_CLOSE(m_FontMaterial);
	SAFE_CLOSE(m_GeomPassMaterial);
	SAFE_CLOSE(m_PointLightPassMaterial);
	SAFE_CLOSE(m_DirLightPassMaterial);
	SAFE_CLOSE(m_NullTech);

	// Other objects
	SAFE_CLOSE(m_Font);
	SAFE_DELETE(m_Camera);
	SAFE_DELETE(m_Gbuffer);
}

//------------------------

/*
Renderer::Renderer() :
	m_Meshes(),
	m_Textures(),
	m_SkyboxMesh(nullptr),
	m_LavaTestMesh(nullptr),
	m_Font(nullptr),
	m_Camera(nullptr),
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
	m_Camera->aspect = static_cast<float>(w / h);

	// TODO : Put this in a function
	m_LightCamera->aspect = static_cast<float>(w / h);
	m_LightCamera->projection = glm::perspective(m_LightCamera->fov, m_LightCamera->aspect, m_LightCamera->near, m_LightCamera->far);

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
	m_Camera = new Camera();
	m_Camera->position = Vec3(0, 1.0f, 4.0f);
	m_Camera->up = Vec3(0.0f, 1.0f, 0.0f);
	m_Camera->right = Vec3(1.0f, 0.0f, 0.0f);
	m_Camera->forward = Vec3(0.0f, 0.0f, -1.0f);
	m_Camera->fov = 45.0f;
	m_Camera->aspect = static_cast<float>(Screen::Instance()->ScreenWidth() / Screen::Instance()->ScreenHeight());
	m_Camera->near = 0.1f;
	m_Camera->far = 500.0f;

	// Light Camera
	m_LightCamera = new Camera();
	m_LightCamera->up = Vec3(0, 1, 0);
	m_LightCamera->near = m_Camera->near;
	m_LightCamera->far = m_Camera->far;
	m_LightCamera->fov = m_Camera->fov;
	m_LightCamera->aspect = m_Camera->aspect;
	m_LightCamera->right = Vec3(1, 0, 0);
	m_LightCamera->position = m_SpotLights[0].Position;
	m_LightCamera->forward = m_SpotLights[0].Direction;
	m_LightCamera->view = glm::lookAt(m_LightCamera->position, m_LightCamera->position + m_LightCamera->forward, m_LightCamera->up);
	m_LightCamera->projection = glm::perspective(m_LightCamera->fov, m_LightCamera->aspect, m_LightCamera->near, m_LightCamera->far);

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
	Mesh* male = new Mesh();
	if (!male->Load("male.obj", true)) { return false; }
	m_Meshes.push_back(male);
	if (!male->AddTexture(MALE_TEX2, 0)) return false;
	if (!male->AddTexture(FAKE_NORMAL_TEX, 0)) return false;
	if (!male->AddTexture(MALE_TEX1, 1)) return false;
	if (!male->AddTexture(FAKE_NORMAL_TEX, 1)) return false;

	Mesh* dino = new Mesh();
	m_Meshes.push_back(dino);
	if (!dino->Load("dino.obj", true)) { return false; }
	if (!dino->AddTexture(DINO_TEX)) return false;	// if no 3rd param then apply to all
	if (!dino->AddTexture(FAKE_NORMAL_TEX)) return false;

	Mesh* cube1 = new Mesh();
	m_Meshes.push_back(cube1);
	if (!cube1->Load("cube.obj", true)) { return false; }
	if (!cube1->AddTexture(BRICK_TEX)) return false;
	if (!cube1->AddTexture(BRICK_NORM_TEX)) return false;

	Mesh* cube2 = new Mesh();
	m_Meshes.push_back(cube2);
	if (!cube2->Load("cube.obj", true)) { return false; }
	if (!cube2->AddTexture(BRICK_TEX)) return false;	
	if (!cube2->AddTexture(FAKE_NORMAL_TEX)) return false;

	// -- This is last for current shadows hack --
	Mesh* floor = new Mesh();
	m_Meshes.push_back(floor);
	if (!floor->Load("cube.obj", true)){return false;}
	if (!floor->AddTexture(WALL_TEX)) return false;
	if (!floor->AddTexture(FAKE_NORMAL_TEX)) return false;

	// This is separate (for now)
	m_SkyboxMesh = new Mesh();
	if (!m_SkyboxMesh->Load("cube.obj", false)) { return false; }
	if (!m_SkyboxMesh->AddTexture(SKYBOX_TEX)) return false;

	// This is separate (for now)
	m_LavaTestMesh = new Mesh();
	if (!m_LavaTestMesh->Load("cube.obj", false)) { return false; }
	if (!m_LavaTestMesh->AddTexture(LAVA_NOISE_TEX)) return false;

	TRANSFORMS.push_back(MALE_XFORM);
	TRANSFORMS.push_back(DINO_XFORM);
	TRANSFORMS.push_back(CUBE1_XFORM);
	TRANSFORMS.push_back(CUBE2_XFORM);
	TRANSFORMS.push_back(FLOOR_XFORM);

	return true;
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
	m_LightCamera->position = m_SpotLights[0].Position;
	m_LightCamera->forward = m_SpotLights[0].Direction;
	m_LightCamera->view = glm::lookAt(m_LightCamera->position, m_LightCamera->position + m_LightCamera->forward, m_LightCamera->up);

	// Here you want to render the meshes that you want to check for shadows
	// We know the floor is last for this hardcoded shit so we dont want that
	for (size_t i = 0; i < m_Meshes.size() - 1; ++i)
	{
		Mesh* thisMesh = m_Meshes[i];
		glBindVertexArray(thisMesh->m_VAO);

		m_ShadowMaterial->setWvpXform(m_LightCamera->projection * m_LightCamera->view * TRANSFORMS[i]);
		m_ShadowMaterial->setTextureUnit(1);

		for (std::vector<SubMesh>::iterator j = thisMesh->m_MeshLayouts.begin(); j != thisMesh->m_MeshLayouts.end(); j++)
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

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::Render()
{
	bool useLighting = true;

	if(DO_SHADOWS)
		ShadowPass();
	
	// Clear color buffer  
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Camera->Update();

	// Render Skybox here, pass camera and mesh
	m_SkyBoxMaterial->Render(m_Camera, m_SkyboxMesh, this);

	if (!useLighting)
	{
		m_DiffuseMaterial->Use();
		m_DiffuseMaterial->setProjXform(m_Camera->projection);
		m_DiffuseMaterial->setViewXform(m_Camera->view);
	}
	else
	{
		m_LightMaterial->Use();

		// Update spot light, to point at male model
		m_SpotLights[0].Position = Vec3(sinf(Time::ElapsedTime() * 2.0f), 5, 5.0f);
		m_SpotLights[0].Direction = Vec3(-0.5f, -1.0f, 0.0f)- m_SpotLights[0].Position;

		m_LightMaterial->setSpotLights(1, m_SpotLights);
		m_LightMaterial->setPointLights(0, nullptr);//m_PointLights);
		m_LightMaterial->setEyeWorldPos(m_Camera->position);
	}

	int i = 0;
	
	// ---- Render Meshes ----
	for (auto mesh = m_Meshes.begin(); mesh != m_Meshes.end(); ++mesh)
	{
		Mesh* thisMesh = (*mesh);
		glBindVertexArray(thisMesh->m_VAO);

		if (!useLighting)
		{
			m_DiffuseMaterial->setModelXform(TRANSFORMS[i]);
		}
		else
		{
			m_LightMaterial->setWorldMatrix(TRANSFORMS[i]);
			m_LightMaterial->setWVP(m_Camera->projection * m_Camera->view * TRANSFORMS[i]);
			
			// This is the object you expect to receive shadows, in this case it's the hard coded floor
			m_LightMaterial->setLightWVP(m_LightCamera->projection * m_LightCamera->view * TRANSFORMS[i]);
			
			// Use this for sampler in light shader
			if(DO_SHADOWS)
				m_ShadowFBO->BindForReading(GL_TEXTURE1);
		}

		for (std::vector<SubMesh>::iterator j = thisMesh->m_MeshLayouts.begin(); j != thisMesh->m_MeshLayouts.end(); j++)
		{
			SubMesh subMesh = (*j);

			for (auto tex = subMesh.TextureIndices.begin(); tex != subMesh.TextureIndices.end(); ++tex)
			{
				m_Textures[ thisMesh->m_TextureHandles[(*tex)]]->Bind();
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
		}

		glBindVertexArray(0);
		++i;
	}

	// Render Lava test mesh
	m_LavaMaterial->Use();
	m_LavaMaterial->setTime(Time::ElapsedTime() * 0.16f);
	m_LavaMaterial->setWvpXform(m_Camera->projection * m_Camera->view * LAVA_XFORM);
	glBindVertexArray(m_LavaTestMesh->m_VAO);
	for (std::vector<SubMesh>::iterator j = m_LavaTestMesh->m_MeshLayouts.begin(); j != m_LavaTestMesh->m_MeshLayouts.end(); j++)
	{
		SubMesh subMesh = (*j);
		for (auto tex = subMesh.TextureIndices.begin(); tex != subMesh.TextureIndices.end(); ++tex)
		{
			m_Textures[m_LavaTestMesh->m_TextureHandles[(*tex)]]->Bind();
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
	}
	glBindVertexArray(0);


	// --- Render New fancy Terrain ----
	if (m_Terrain)
	{
		m_Terrain->Render(this, m_Camera, Vec4(1.0f));
	}

	// ---- Render Billboards ----
	if (m_TreeBillboardList)
	{
		m_TreeBillboardList->Render(this, m_Camera->projection * m_Camera->view, m_Camera->position, m_Camera->right);
	}

	// -- Render Text ----
	RenderText("CGR Engine", 8, 16);
	RenderText("Cam Fwd: " + util::vec3_to_str(m_Camera->forward), 8, 32);
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
	SAFE_DELETE(m_SkyboxMesh);
	SAFE_DELETE(m_LavaTestMesh);
	m_Meshes.clear();

	// Clear Textures
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		SAFE_DELETE(m_Textures[i]);
	}
	m_Textures.clear();

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
	SAFE_DELETE(m_Camera);
	SAFE_DELETE(m_LightCamera);
	SAFE_DELETE(m_ShadowFBO);
	SAFE_DELETE(m_TreeBillboardList);
	SAFE_DELETE(m_Terrain);
}
*/
