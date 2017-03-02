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

#include <ft2build.h>
#include FT_FREETYPE_H

// TODO : Remove this tragedy
const Mat4 MALE_XFORM = glm::translate(Mat4(1.0f), Vec3(-0.5f, -1.5f, 0.0f)) *
	glm::scale(Mat4(1.0f), Vec3(1.0f));
const Mat4 DINO_XFORM = glm::translate(Mat4(1.0f), Vec3(1.8f, -1.5f, 0.0f)) *
	glm::scale(Mat4(1.0f), Vec3(0.05f));
const Mat4 FLOOR_XFORM = glm::translate(Mat4(1.0f), Vec3(0.0f, -2.0f, -3.0f)) *
	glm::scale(Mat4(1.0f), Vec3(30.0f, 0.5f, 30.0f));

std::vector<Mat4> TRANSFORMS;


Renderer::Renderer() :
	m_Meshes(),
	m_Textures(),
	m_SkyboxMesh(nullptr),
	m_Font(nullptr),
	m_Camera(nullptr),
	m_LightCamera(nullptr),
	m_ShadowFBO(nullptr),
	m_FontMaterial(nullptr),
	m_LightMaterial(nullptr),
	m_DiffuseMaterial(nullptr),
	m_SkyBoxMaterial(nullptr),
	m_ShadowMaterial(nullptr),
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

	return success;
}

bool Renderer::setRenderStates()
{
	// ** These states could differ **
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS);
	//glEnable(GL_CULL_FACE);

	return true;
}

bool Renderer::setFrameBuffers()
{
	m_ShadowFBO = new ShadowFrameBuffer();
	if (!m_ShadowFBO->Init(Screen::Instance()->FrameBufferWidth(), Screen::Instance()->FrameBufferHeight()))
	{
		return false;
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
	if (m_ShadowFBO)
	{
		m_ShadowFBO->Init(w, h);
	}
}

bool Renderer::setLights()
{
	// Dir Light
	m_DirectionalLight.Color = Vec3(1.0f, 1.0f, 1.0f);
	m_DirectionalLight.AmbientIntensity = 0.05f;
	m_DirectionalLight.DiffuseIntensity = 0.01f;
	m_DirectionalLight.Direction = Vec3(1.0f, -1.0f, 0.0f);

	/*
	// Point Lights
	m_PointLights[0].Color = Vec3(1.0f, 1.0f, 1.0f);
	m_PointLights[0].DiffuseIntensity = 0.5f;
	m_PointLights[0].Position = Vec3(-7.0f, 5.0f, -7.0f);
	m_PointLights[0].Attenuation.Linear = 0.1f;

	m_PointLights[1].Color = Vec3(1.0f, 1.0f, 1.0f);
	m_PointLights[1].DiffuseIntensity = 0.5f;
	m_PointLights[1].Attenuation.Linear = 0.1f;
	m_PointLights[1].Position = Vec3(7.0f, 5.0f, 7.0f);
	*/

	// Spot Lights
	m_SpotLights[0].DiffuseIntensity = 0.9f;
	m_SpotLights[0].Color = Vec3(1.0f, 1.0f, 1.0f);
	m_SpotLights[0].Attenuation.Linear = 0.1f;
	m_SpotLights[0].Cutoff = Maths::ToRadians(30.0f);
	m_SpotLights[0].Direction = Vec3(-0.1f, -1, 0.0f);
	m_SpotLights[0].Position = Vec3(2, 5, 5.0f);
	m_SpotLights[0].Direction = Vec3(-0.5f, -1.0f, 0.0f)/*male mesh pos*/ - m_SpotLights[0].Position;

	/*
	m_SpotLights[1].DiffuseIntensity = 0.9f;
	m_SpotLights[1].Color = Vec3(1.0f, 1.0f, 1.0f);
	m_SpotLights[1].Attenuation.Linear = 0.1f;
	m_SpotLights[1].Cutoff = Maths::ToRadians(100.0f);
	m_SpotLights[1].Direction = Vec3(0.1f, -1, -0.1f);
	*/

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

bool Renderer::loadTetxures()
{
	// ---- Images ----
	Image maleImg;
	if (!maleImg.LoadImg("../resources/textures/male_body_low_albedo.tga"))
	{
		WRITE_LOG("Failed to load wall texture", "error");
		return false;
	}

	Image maleImg2;
	if (!maleImg2.LoadImg("../resources/textures/male_body_high_albedo.tga"))
	{
		WRITE_LOG("Failed to load wall texture", "error");
		return false;
	}

	Image dinoImg;
	if (!dinoImg.LoadImg("../resources/textures/dino_diffuse.tga"))
	{
		WRITE_LOG("Failed to load wall texture", "error");
		return false;
	}

	Image wallImg;
	if (!wallImg.LoadImg("../resources/textures/wall.tga"))
	{
		WRITE_LOG("Failed to load wall texture", "error");
		return false;
	}

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

	// ---- Textures ----
	Texture* maleTex2 = new Texture(GL_TEXTURE_2D, GL_TEXTURE0);
	Texture* maleTex = new Texture(GL_TEXTURE_2D, GL_TEXTURE0);
	Texture* dinoTex = new Texture(GL_TEXTURE_2D, GL_TEXTURE0);
	Texture* wallTex = new Texture(GL_TEXTURE_2D, GL_TEXTURE0);
	Texture* cubeMapTex = new Texture(GL_TEXTURE_CUBE_MAP, GL_TEXTURE0);

	m_Textures.push_back(maleTex);
	m_Textures.push_back(maleTex2);
	m_Textures.push_back(dinoTex);
	m_Textures.push_back(wallTex);
	m_Textures.push_back(cubeMapTex);

	if (!maleTex2->Create(&maleImg2)) return false;
	if (!maleTex->Create(&maleImg)) return false;
	if (!dinoTex->Create(&dinoImg))return false;
	if (!wallTex->Create(&wallImg))return false;
	if (!cubeMapTex->Create(images))return false;

	return true;
}

bool Renderer::loadMeshes()
{
	const size_t MALE_TEX1 = 0;
	const size_t MALE_TEX2 = 1;
	const size_t DINO_TEX = 2;
	const size_t WALL_TEX = 3;
	const size_t CUBE_TEX = 4;

	// ---- Load Meshes ----
	Mesh* male = new Mesh();
	if (!male->Load("male.obj")) { SAFE_DELETE(male); return false; }

	// Create Mat
	m_Meshes.push_back(male);
	if (!male->AddTexture(MALE_TEX2, 0)) return false;
	if (!male->AddTexture(MALE_TEX1, 1)) return false;

	Mesh* dino = new Mesh();
	if (!dino->Load("dino.obj")) { SAFE_DELETE(dino); return false; }

	if (!dino->AddTexture(DINO_TEX)) return false;	// if no 3rd param then apply to all
	m_Meshes.push_back(dino);

	Mesh* cube = new Mesh();
	if (!cube->Load("cube.obj"))
	{
		SAFE_DELETE(cube);
		return false;
	}

	if (!cube->AddTexture(WALL_TEX)) return false;	// if no 3rd param then apply to all
	m_Meshes.push_back(cube);

	m_SkyboxMesh = new Mesh();
	if (!m_SkyboxMesh->Load("cube.obj"))
	{
		return false;
	}

	if (!m_SkyboxMesh->AddTexture(CUBE_TEX)) return false;

	TRANSFORMS.push_back(MALE_XFORM);
	TRANSFORMS.push_back(DINO_XFORM);
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

	if(useLighting)
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
	}
	else
	{
		m_LightMaterial->Use();

		// Update spot light, to point at male model
		m_SpotLights[0].Position = Vec3(sinf(Time::ElapsedTime() * 2.0f), 5, 5.0f);
		m_SpotLights[0].Direction = Vec3(-0.5f, -1.0f, 0.0f)/*male mesh pos*/ - m_SpotLights[0].Position;

		m_LightMaterial->setSpotLights(1, m_SpotLights);
		m_LightMaterial->setPointLights(0, nullptr);//m_PointLights);
		m_LightMaterial->setEyeWorldPos(m_Camera->position);
	}

	// Uniforms
	if (!useLighting)
	{
		m_DiffuseMaterial->setProjXform(m_Camera->projection);
		m_DiffuseMaterial->setViewXform(m_Camera->view);
	}

	int i = 0;
	// ---- Render ----
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
			m_ShadowFBO->BindForReading(GL_TEXTURE1);
		}

		for (std::vector<SubMesh>::iterator j = thisMesh->m_MeshLayouts.begin(); j != thisMesh->m_MeshLayouts.end(); j++)
		{
			SubMesh subMesh = (*j);

			if (subMesh.TextureIndex != INVALID_TEXTURE_LOCATION)
			{
				m_Textures[thisMesh->m_TextureHandles[subMesh.TextureIndex]]->Bind();
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

	RenderText("CGR Engine", 8, 16);
	RenderText("Cam Fwd: " + util::vec3_to_str(m_Camera->forward), 8, 32);
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
	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		SAFE_DELETE(m_Meshes[i]);
	}
	SAFE_DELETE(m_SkyboxMesh);
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
	SAFE_CLOSE(m_SkyBoxMaterial);

	// Other objects
	SAFE_CLOSE(m_Font);
	SAFE_DELETE(m_Camera);
	SAFE_DELETE(m_LightCamera);
	SAFE_DELETE(m_ShadowFBO);
}
