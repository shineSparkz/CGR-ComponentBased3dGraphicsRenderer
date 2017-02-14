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

#include <ft2build.h>
#include FT_FREETYPE_H

bool load_shader(const std::string& srcPath, GLuint& shaderOut, GLenum shaderType)
{
	TextFile tf;
	std::string shadersrc = tf.LoadFileIntoStr(srcPath);
	if (shadersrc == "")
	{
		return false;
	}
	// Need to cast to a c string for open GL
	const char* src = shadersrc.c_str();

	shaderOut = glCreateShader(shaderType);
	if (OpenGLLayer::check_GL_error())
	{
		WRITE_LOG("Error: Invalid shader type", "error");
		return false;
	}

	// Ask OpenGL to attempt shader compilation
	GLint compile_status = 0;
	glShaderSource(shaderOut, 1, (const GLchar**)&src, NULL);
	glCompileShader(shaderOut);
	glGetShaderiv(shaderOut, GL_COMPILE_STATUS, &compile_status);

	if (compile_status != GL_TRUE)
	{
		// Log what went wrong in shader src
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(shaderOut, string_length, NULL, log);
		std::stringstream logger;
		logger << log << std::endl;
		WRITE_LOG(logger.str(), "error");
		return false;
	}

	return true;
}


Renderer::Renderer()
{
}

bool Renderer::Init()
{
	m_Font = new Font();

	if (!m_Font->CreateFont("../resources/fonts/cour.ttf", 24))
	{
		WRITE_LOG("FONT LOAD FAIL", "error");
		return false;
	}


	m_FontTechnique = new FontTechnique();
	if (!m_FontTechnique->Init())
	{
		return false;
	}

	/*
	GLuint vertShader, fragShader;

	if (!load_shader("../resources/shaders/font_vs.glsl", vertShader, GL_VERTEX_SHADER))
	{
		OpenGLLayer::clean_GL_shader(&vertShader);
		return false;
	}

	if (!load_shader("../resources/shaders/font_fs.glsl", fragShader, GL_FRAGMENT_SHADER))
	{
		OpenGLLayer::clean_GL_shader(&vertShader);
		OpenGLLayer::clean_GL_shader(&fragShader);
		return false;
	}

	m_FontShaderProg = glCreateProgram();
	glAttachShader(m_FontShaderProg, vertShader);
	glAttachShader(m_FontShaderProg, fragShader);

	// **** Dynamic ****
	glBindAttribLocation(m_FontShaderProg, 0, "vertex_position");
	glBindFragDataLocation(m_FontShaderProg, 0, "frag_colour");

	glLinkProgram(m_FontShaderProg);

	GLint link_status = 0;
	glGetProgramiv(m_FontShaderProg, GL_LINK_STATUS, &link_status);

	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(m_FontShaderProg, string_length, NULL, log);
		std::stringstream logger;
		logger << log << std::endl;
		WRITE_LOG(logger.str(), "error");
		OpenGLLayer::clean_GL_shader(&vertShader);
		OpenGLLayer::clean_GL_shader(&fragShader);
	}

	OpenGLLayer::clean_GL_shader(&vertShader);
	OpenGLLayer::clean_GL_shader(&fragShader);
	*/

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

	// Dir Light
	m_directionalLight.Color = Vec3(1.0f, 1.0f, 1.0f);
	m_directionalLight.AmbientIntensity = 0.05f;
	m_directionalLight.DiffuseIntensity = 0.01f;
	m_directionalLight.Direction = Vec3(1.0f, 1.0f, 0.0f);

	// Point Lights
	m_PointLights[0].DiffuseIntensity = 0.25f;
	m_PointLights[0].Color = Vec3(1.0f, 0.5f, 0.0f);
	m_PointLights[0].Attenuation.Linear = 0.1f;
	m_PointLights[1].DiffuseIntensity = 0.25f;
	m_PointLights[1].Color = Vec3(0.0f, 0.5f, 1.0f);
	m_PointLights[1].Attenuation.Linear = 0.1f;

	// Spot Lights
	m_SpotLights[0].DiffuseIntensity = 0.9f;
	m_SpotLights[0].Color = Vec3(0.0f, 1.0f, 1.0f);
	m_SpotLights[0].Attenuation.Linear = 0.1f;
	m_SpotLights[0].Cutoff = 10.0f;
	m_SpotLights[1].DiffuseIntensity = 0.9f;
	m_SpotLights[1].Color = Vec3(1.0f, 1.0f, 1.0f);
	m_SpotLights[1].Direction = Vec3(-0.1f, -1.0f, 0.0f);
	m_SpotLights[1].Attenuation.Linear = 0.1f;
	m_SpotLights[1].Cutoff = 30.0f;

	m_pEffect = new LightingTechnique();
	if (!m_pEffect->Init())
	{
		WRITE_LOG("Light effect error", "error");
		return false;
	}
	//m_pEffect->Enable();
	m_pEffect->SetTextureUnit(0);

	m_DiffuseMat = new BasicDiffuseTechnique();
	if (!m_DiffuseMat->Init())
	{
		WRITE_LOG("Failed to create diffuse mat", "error");
	}

	// ** These states could differ **
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS);
	glEnable(GL_CULL_FACE);

	//ReloadShaders();

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

	// ---- Textures ----
	Texture* maleTex2 = new Texture(GL_TEXTURE_2D, GL_TEXTURE0);
	Texture* maleTex = new Texture(GL_TEXTURE_2D, GL_TEXTURE0);
	Texture* dinoTex = new Texture(GL_TEXTURE_2D, GL_TEXTURE0);
	Texture* wallTex = new Texture(GL_TEXTURE_2D, GL_TEXTURE0);
	m_Textures.push_back(maleTex);
	m_Textures.push_back(maleTex2);
	m_Textures.push_back(dinoTex);
	m_Textures.push_back(wallTex);
	if (!maleTex2->Create(&maleImg2)) return false;
	if (!maleTex->Create(&maleImg)) return false;
	if (!dinoTex->Create(&dinoImg))return false;
	if (!wallTex->Create(&wallImg))return false;

	const size_t MALE_TEX1 = 0;
	const size_t MALE_TEX2 = 1;
	const size_t DINO_TEX =  2;
	const size_t WALL_TEX =  3;


	// ---- Load Meshes ----
	Mesh* male = new Mesh();
	if (!male->Load("male.obj")){ SAFE_DELETE(male); return false;}

	// Create Mat
	m_Meshes.push_back(male);
	if (!male->AddTexture(MALE_TEX2, 0)) return false;
	if (!male->AddTexture(MALE_TEX1,  1)) return false;

	Mesh* dino = new Mesh();
	if (!dino->Load("dino.obj")){ SAFE_DELETE(dino); return false;}

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

void Renderer::Render()
{
	// Clear color buffer  
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Camera->Update();

	bool useLighting = true;
	

	if (!useLighting)
	{
		m_DiffuseMat->Use();
	}
	else
	{
		m_pEffect->Enable();
		// ---- Set Light shader uniforms ----
		m_PointLights[0].Position = Vec3(3.0f, 1.0f, 10.0f * (cosf(2.0f) + 1.0f) / 2.0f);
		m_PointLights[1].Position = Vec3(7.0f, 2.0f, 10.0f * (sinf(2.0f) + 1.0f) / 2.0f);
		m_pEffect->SetPointLights(2, m_PointLights);

		m_SpotLights[0].Position = m_Camera->position;
		m_SpotLights[0].Direction = m_Camera->position + m_Camera->forward;
		m_SpotLights[1].Position = Vec3(sinf(Time::ElapsedTime()), 2.0f, -1.0f);
		m_pEffect->SetSpotLights(2, m_SpotLights);

		m_pEffect->SetDirectionalLight(m_directionalLight);
		m_pEffect->SetEyeWorldPos(m_Camera->position);
		m_pEffect->SetMatSpecularIntensity(0.2f);
		m_pEffect->SetMatSpecularPower(0.2f);
	}

	// Would get the transformation matrix from each object that needed rendering
	Mat4 malemodelMat =
		glm::translate(Mat4(1.0f), Vec3(-0.5f, -1.0f, 0.0f)) *
		//( Maths::RotateY(cosf(Time::ElapsedTime() * 0.1f) * 3.0f) ) *
		glm::scale(Mat4(1.0f), Vec3(1.0f));

	Mat4 dinomodelMat =
		glm::translate(Mat4(1.0f), Vec3(0.8f, -1.0f, 0.0f)) *
		//(Maths::RotateY(sinf(Time::ElapsedTime() * 0.1f) * 3.0f)) *
		glm::scale(Mat4(1.0f), Vec3(0.05f));

	Mat4 floorMat =
		glm::translate(Mat4(1.0f), Vec3(0.0f, -2.0f, -3.0f)) *
		glm::scale(Mat4(1.0f), Vec3(30.0f, 0.5f, 30.0f));

	// Uniforms
	if (!useLighting)
	{
		m_DiffuseMat->setProjXform(m_Camera->projection);
		m_DiffuseMat->setViewXform(m_Camera->view);
	}

	int i = 0;
	// ---- Render ----
	for (auto mesh = m_Meshes.begin(); mesh != m_Meshes.end(); ++mesh)
	{
		Mesh* thisMesh = (*mesh);
		glBindVertexArray(thisMesh->m_VAO);

		if (i == 0)
		{
			if (!useLighting)
			{
				m_DiffuseMat->setModelXform(malemodelMat);
			}
			else
			{
				m_pEffect->SetWorldMatrix(malemodelMat);
				m_pEffect->SetWVP(m_Camera->projection * m_Camera->view * malemodelMat);
			}
		}
		else if(i == 1)
		{
			if (!useLighting)
			{
				m_DiffuseMat->setModelXform(dinomodelMat);
			}
			else
			{
				m_pEffect->SetWVP(m_Camera->projection * m_Camera->view * dinomodelMat);
				m_pEffect->SetWorldMatrix(dinomodelMat);
			}
		}
		else
		{
			if (!useLighting)
			{
				m_DiffuseMat->setModelXform(floorMat);
			}
			else
			{
				m_pEffect->SetWVP(m_Camera->projection * m_Camera->view * floorMat);
				m_pEffect->SetWorldMatrix(floorMat);
			}
		}
		++i;

		for (std::vector<SubMesh>::iterator i = thisMesh->m_MeshLayouts.begin();
			i != thisMesh->m_MeshLayouts.end(); i++)
		{
			SubMesh subMesh = (*i);

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
	}

	RenderText("CGR Engine", 8, 16);
	//RenderText("Cam Fwd: " + util::vec3_to_str(m_Camera->forward), 8, 16);
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
	m_FontTechnique->Use();
	//glUseProgram(m_FontShaderProg);

	Mat4 projection = glm::ortho(0.0f, (float)Screen::Instance()->FrameBufferWidth(), 
		0.0f, 
		(float)Screen::Instance()->FrameBufferHeight());

	
	m_FontTechnique->setProjection(projection);
	m_FontTechnique->setColour(colour.Normalize());
	//glUniformMatrix4fv(glGetUniformLocation(m_FontShaderProg, "proj_xform"), 1, GL_FALSE, glm::value_ptr(projection));
	//glUniform4fv(glGetUniformLocation(m_FontShaderProg, "text_colour"),1, glm::value_ptr(colour.Normalize()));

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(m_Font->m_Vao);

	// Iterate through all characters
	std::string::const_iterator i;
	for (i = txt.begin(); i != txt.end(); ++i)
	{
		Character ch = m_Font->m_Characters[(*i)];

		float xpos = x + ch.bearingX;
		float ypos = y - (ch.sizeY - ch.bearingY);

		float w = ch.sizeX;
		float h = ch.sizeY;

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
	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		SAFE_DELETE(m_Meshes[i]);
	}

	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		SAFE_DELETE(m_Textures[i]);
	}

	m_Meshes.clear();
	m_Textures.clear();

	SAFE_CLOSE(m_FontTechnique);
	SAFE_CLOSE(m_DiffuseMat);

	SAFE_CLOSE(m_Font);
	SAFE_CLOSE(m_pEffect);

	SAFE_DELETE(m_Camera);
}
