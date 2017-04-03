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
#include "ShaderProgram.h"
#include "ResourceManager.h"
#include "UniformBlockManager.h"
#include "UniformBlock.h"

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
float ATTEN_QUAD = 0.000080f;

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
	m_ResManager(nullptr),
	m_CameraPtr(nullptr),
	m_Query(),
	m_QueryTime(0),
	m_Frames(0),
	m_Gbuffer(nullptr),
	m_TreeBillboardList(nullptr),
	m_Terrain(nullptr),
	m_UniformBlockManager(nullptr)
{
}

bool Renderer::Init()
{
	m_Query.Init(GL_TIME_ELAPSED);
	
	bool success = true;

	// This needs to be before this as shaders depend on it
	success &= createUniformBlocks();
	
	m_ResManager = new ResourceManager();
	success &= m_ResManager->CreateDefaultResources();

	success &= setFrameBuffers();
	success &= setLights();

	// Create Terrain
	std::vector<Vec3> billboardPositions;
	m_Terrain = new Terrain();

	unsigned int textures[5] = { TERRAIN1_TEX, TERRAIN2_TEX, TERRAIN3_TEX, TERRAIN4_TEX, TERRAIN5_TEX };
	success &= m_Terrain->LoadFromHeightMapWithBillboards(
		"../resources/textures/terrain/heightmap.tga",
		m_ResManager->GetShader(TERRAIN_SHADER),
		textures,
		Vec3(200, 30, 200),
		billboardPositions,
		500
	);

	// Need material and textures for bill board creation, which again I am not too happy with
	m_TreeBillboardList = new BillboardList();
	success &= m_TreeBillboardList->InitWithPositions(m_ResManager->GetShader(BILLBOARD_SHADER), TREE_BILLBOARD_TEX, 0.5f, billboardPositions);

	/*
	m_TreeBillboardList->Init(m_ResManager->GetShader(BILLBOARD_SHADER), TREE_BILLBOARD_TEX, 
		0.5f,	// scale
		10,		// numX
		10,		// numY
		2.0f,	// spacing
		14.0f,	// offset pos
		-1.4f	// ypos
	);
	*/
	
	return success;
}

bool Renderer::createUniformBlocks()
{
	if(!m_UniformBlockManager)
		m_UniformBlockManager = new UniformBlockManager();
	
	std::vector<std::string> names;

	//===================   Scene Block  ==================================
	names.push_back("camera_position");
	names.push_back("ambient_light");
	names.push_back("view_xform");
	names.push_back("proj_xform");
	if (!m_UniformBlockManager->CreateBlock("scene", names))
		return false;


	//===================   Lights Block  ================================
	names.clear();

	// Dir Light
	names.push_back("directionLight.direction");
	names.push_back("directionLight.intensity");
	
	// Counters
	names.push_back("numPoints");
	names.push_back("numSpots");

	// Point Lights
	for (int i = 0; i < MAX_POINTS; ++i)
	{
		names.push_back("pointLights[" + std::to_string(i) + "].position");
		names.push_back("pointLights[" + std::to_string(i) + "].intensity");
		names.push_back("pointLights[" + std::to_string(i) + "].ambient_intensity");
		names.push_back("pointLights[" + std::to_string(i) + "].aConstant");
		names.push_back("pointLights[" + std::to_string(i) + "].aLinear");
		names.push_back("pointLights[" + std::to_string(i) + "].aQuadratic");
	}

	// Spot Lights
	for (int i = 0; i < MAX_SPOTS; ++i)
	{
		names.push_back("spotLights[" + std::to_string(i) + "].position");
		names.push_back("spotLights[" + std::to_string(i) + "].direction");
		names.push_back("spotLights[" + std::to_string(i) + "].intensity");
		names.push_back("spotLights[" + std::to_string(i) + "].coneAngle");
		names.push_back("spotLights[" + std::to_string(i) + "].aConstant");
		names.push_back("spotLights[" + std::to_string(i) + "].aLinear");
		names.push_back("spotLights[" + std::to_string(i) + "].aQuadratic");
		names.push_back("spotLights[" + std::to_string(i) + "].switched_on");
	}

	if (!m_UniformBlockManager->CreateBlock("Lights", names))
		return false;

	return true;
}

bool Renderer::SetCamera(BaseCamera* camera)
{
	m_CameraPtr = camera;
	return true;
}

void Renderer::Render(std::vector<GameObject*>& gameObjects)
{
	// Temp
	if (Mouse::Instance()->LMB)
		DIRECTION_ANGLE += 1.0f;
	if (Mouse::Instance()->RMB)
	{
		m_SpotLights[0].ToggleLight();
	}

	m_Query.Start();

	// Set Light data as uniform buffer

	if (m_DeferredRender)
		deferredRender(gameObjects);
	else
		forwardRender(gameObjects);

	m_Query.End();

	if (++m_Frames > 2)
	{
		m_QueryTime = m_Query.Result(false);
		m_Frames = 0;
	}

	// -- Render Text ----
	RenderText(FONT_COUR, "Frm Time: " + util::to_str(getFrameTime(TimeMeasure::Seconds)), 8, Screen::Instance()->FrameBufferHeight() - 16.0f, FontAlign::Left, Colour::Red());

}

void Renderer::RenderText(size_t fontId, const std::string& txt, float x, float y, FontAlign fa, const Colour& colour)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate corresponding render state	
	m_ResManager->m_Shaders[FONT_SHADER]->Use();

	Mat4 projection = glm::ortho(0.0f, (float)Screen::Instance()->FrameBufferWidth(),
		0.0f,
		(float)Screen::Instance()->FrameBufferHeight());

	m_ResManager->m_Shaders[FONT_SHADER]->SetUniformValue<Mat4>("u_proj_xform", &projection);
	m_ResManager->m_Shaders[FONT_SHADER]->SetUniformValue<Vec4>("text_colour", &colour.Normalize());

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(m_ResManager->m_Fonts[fontId]->m_Vao);

	// Iterate through all characters
	std::string::const_iterator i;
	for (i = txt.begin(); i != txt.end(); ++i)
	{
		Character ch = m_ResManager->m_Fonts[fontId]->m_Characters[(*i)];

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
		glBindBuffer(GL_ARRAY_BUFFER, m_ResManager->m_Fonts[fontId]->m_Vbo);
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

void Renderer::WindowSizeChanged(int w, int h)
{
	m_CameraPtr->SetAspect(static_cast<float>(w / h));

	if (m_Gbuffer)
	{
		m_Gbuffer->Init();
	}
}

void Renderer::Close()
{
	m_Query.Clean();

	SAFE_DELETE(m_TreeBillboardList);
	SAFE_DELETE(m_Terrain);
	SAFE_DELETE(m_Gbuffer);

	SAFE_CLOSE(m_UniformBlockManager);
	SAFE_CLOSE(m_ResManager);
}

size_t Renderer::GetNumSubMeshesInMesh(size_t meshIndex) const
{
	return m_ResManager->GetNumSubMeshesInMesh(meshIndex);
}

size_t Renderer::GetNumTextures() const
{
	return m_ResManager->GetNumTextures();
}

Texture* Renderer::GetTexture(size_t index) const
{
	return m_ResManager->GetTexture(index);
}


void Renderer::forwardRender(std::vector<GameObject*>& gameObjects)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!m_CameraPtr)
		// Should log if that hasn't been set
		return;

	if (m_CameraPtr->HasSkybox())
	{
		this->renderSkybox(m_CameraPtr);
	}

	// Uniform Blocks
	UniformBlock* scene = m_UniformBlockManager->GetBlock("scene");
	scene->SetValue("ambient_light", (void*)glm::value_ptr(AMBIENT_LIGHT));
	scene->SetValue("view_xform", (void*)glm::value_ptr(m_CameraPtr->View()));
	scene->SetValue("proj_xform", (void*)glm::value_ptr(m_CameraPtr->Projection()));
	scene->Bind();

	UniformBlock* lights = m_UniformBlockManager->GetBlock("Lights");
	if (lights)
	{
		// Dir Light Update
		m_DirLight.direction.y = sinf(Time::ElapsedTime());
		lights->SetValue("directionLight.direction", (void*)glm::value_ptr(m_DirLight.direction));
		lights->SetValue("directionLight.intensity", (void*)glm::value_ptr(m_DirLight.intensity));

		// Update Point Light
		float light_val_x = sinf(Time::ElapsedTime()) * 8.0f;
		float light_val_z = cosf(Time::ElapsedTime()) * 8.0f;
		for (int i = 0; i < 1; ++i)
		{
			m_PointLights[i].position =
				Vec3(light_val_x,
					m_PointLights[i].position.y,
					light_val_z);

			lights->SetValue("pointLights[" + std::to_string(i) + "].position", &m_PointLights[i].position);
		}

		// Update Cam spot light
		m_SpotLights[0].direction = m_CameraPtr->Forward();
		m_SpotLights[0].position = m_CameraPtr->Position();
		lights->SetValue("spotLights[" + std::to_string(0) + "].position", &m_SpotLights[0].position);
		lights->SetValue("spotLights[" + std::to_string(0) + "].direction", &m_SpotLights[0].direction);
		lights->SetValue("spotLights[" + std::to_string(0) + "].switched_on", &m_SpotLights[0].switched_on);

		lights->Bind();
	}

	ShaderProgram* sp = m_ResManager->m_Shaders[STD_FWD_LIGHTING]; //m_Shaders[mr->m_ShaderIndex];
	if (sp)
	{
		sp->Use();
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
			this->renderMesh(mr);
		}
	}

	// Render Terrain
	//m_Terrain->Render(this, m_CameraPtr, Vec3(1.0f));
	//m_TreeBillboardList->Render(this, m_CameraPtr->ProjXView(), m_CameraPtr->Position());
}

void Renderer::deferredRender(std::vector<GameObject*>& gameObjects)
{
	m_Gbuffer->StartFrame();

	// Geom Pass
	{
		// Skybox
		if (m_CameraPtr->HasSkybox())
			renderSkybox(m_CameraPtr);

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

			ShaderProgram* sp = m_ResManager->m_Shaders[mr->m_ShaderIndex];
			if (sp)
			{
				sp->Use();
				sp->SetUniformValue<Mat4>("u_WVP", &(ProjViewXform * t->GetModelXform()));
				sp->SetUniformValue<Mat4>("u_World", &(t->GetModelXform()));

				// This is shit
				float elapsed = Time::ElapsedTime();
				sp->SetUniformValue<float>("u_GlobalTime", &elapsed);

				// Render Mesh here
				this->renderMesh(mr);
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
				m_ResManager->m_Shaders[STD_DEF_STENCIL_SHADER]->Use();

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
				m_ResManager->m_Shaders[STD_DEF_STENCIL_SHADER]->SetUniformValue<Mat4>("u_WVP", &(m_CameraPtr->Projection() * m_CameraPtr->View() * LIGHT_TRANS));
				this->renderMesh(m_ResManager->m_Meshes[SPHERE_MESH]);
			}

			// Point Light
			{
				m_Gbuffer->BindForLightPass();

				ShaderProgram* sp = m_ResManager->m_Shaders[STD_DEF_PNT_LIGHT_SHADER];
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

				this->renderMesh(m_ResManager->m_Meshes[SPHERE_MESH]);

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
		m_ResManager->m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->Use();
		m_ResManager->m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<Vec3>("u_EyeWorldPos", &m_CameraPtr->Position());
		// Should only set once
		m_ResManager->m_Shaders[STD_DEF_DIR_LIGHT_SHADER]->SetUniformValue<Mat4>("u_WVP", &(Mat4(1.0f)));

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		this->renderMesh(m_ResManager->m_Meshes[QUAD_MESH]);
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

void Renderer::renderMesh(Mesh* thisMesh)
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

void Renderer::renderMesh(MeshRenderer* meshInstance)
{
	Mesh* thisMesh = m_ResManager->m_Meshes[meshInstance->MeshIndex];
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
				m_ResManager->m_Textures[meshInstance->m_TextureHandles[(*tex)]]->Bind();
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

void Renderer::renderSkybox(BaseCamera* cam)
{
	if(m_DeferredRender)
		glEnable(GL_DEPTH_TEST);
	
	// Use skybox material
	m_ResManager->m_Shaders[SKYBOX_SHADER]->Use();

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

	m_ResManager->m_Shaders[SKYBOX_SHADER]->SetUniformValue<Mat4>("world_xform", &(model));

	// Render mesh with texture here, THIS SHOULDNT BE HERE
	glBindVertexArray(m_ResManager->m_Meshes[CUBE_MESH]->m_VAO);
	Texture* t = m_ResManager->m_Textures[sb->textureIndex];
	if (t) t->Bind();

	for (std::vector<SubMesh>::iterator i = m_ResManager->m_Meshes[CUBE_MESH]->m_SubMeshes.begin();
		i != m_ResManager->m_Meshes[CUBE_MESH]->m_SubMeshes.end(); i++)
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

bool Renderer::setFrameBuffers()
{
	if (m_DeferredRender)
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
	}

	return true;
}

bool Renderer::setLights()
{
	// TODO ::: THESE LIGHTS SHOULD BE PASSED IN BY THE SCENE

	// Dir Light
	m_DirLight.direction = Vec3(0, -1, 0);
	m_DirLight.intensity = Vec3(0.7f, 0.7f, 0.7f);
	m_DirLight.ambient_intensity = 0.1f;

	// Point Light
	m_PointLights.resize(3);

	m_PointLights[0].position = Vec3(20.0f, 3.0f, -20.0f);
	m_PointLights[1].position = Vec3(-50.0f, 3.0f, 30.0f);
	m_PointLights[2].position = Vec3(70.0f, 3.0f, 10.0f);
	m_PointLights[0].intensity = Vec3(0.7f, 0.7f, 0.7f);
	m_PointLights[1].intensity = Vec3(0.7f, 0.7f, 0.7f);
	m_PointLights[2].intensity = Vec3(0.7f, 0.7f, 0.7f);

	for (size_t i = 0; i < m_PointLights.size(); ++i)
	{
		m_PointLights[i].ambient_intensity = 0.2f;
		m_PointLights[i].aConstant = ATTEN_CONST;
		m_PointLights[i].aLinear = ATTEN_LIN;
		m_PointLights[i].aQuadratic = ATTEN_QUAD;
	}

	// Spot Light(s)
	m_SpotLights.resize(1);
	m_SpotLights[0].position = Vec3(0.0f);
	m_SpotLights[0].direction = Vec3(0.0f, 0.0f, -1.0f);
	m_SpotLights[0].switched_on = 1;
	m_SpotLights[0].intensity = Vec3(0.2f, 0.2f, 0.6f);
	m_SpotLights[0].aConstant = ATTEN_CONST;
	m_SpotLights[0].aLinear = 0.0174f;
	m_SpotLights[0].aQuadratic = ATTEN_QUAD;
	m_SpotLights[0].SetAngle(15.0f);
	//----------------------------------

	// TODO : change this on window size callback
	Vec2 screenSize((float)Screen::Instance()->FrameBufferWidth(), (float)Screen::Instance()->FrameBufferHeight());
	int sampler = 0;
	int num_points = static_cast<int>(m_PointLights.size());
	int num_spots =  static_cast<int>(m_SpotLights.size());

	// Set Light Uniforms
	ShaderProgram* lsp = m_ResManager->GetShader(STD_FWD_LIGHTING);
	lsp->Use();

	// Set constants
	lsp->SetUniformValue<int>("u_sampler", &sampler);

	// Light Uniform Block
	UniformBlock* lights = m_UniformBlockManager->GetBlock("Lights");
	if (lights)
	{
		lights->SetValue("numPoints", &num_points);
		lights->SetValue("numSpots", &num_spots);

		// Point Lights
		for (int i = 0; i < num_points; ++i)
		{
			lights->SetValue("pointLights[" + std::to_string(i) + "].position", &m_PointLights[i].position);
			lights->SetValue("pointLights[" + std::to_string(i) + "].intensity", &m_PointLights[i].intensity);
			lights->SetValue("pointLights[" + std::to_string(i) + "].ambient_intensity", &m_PointLights[i].ambient_intensity);
			lights->SetValue("pointLights[" + std::to_string(i) + "].aConstant", &m_PointLights[i].aConstant);
			lights->SetValue("pointLights[" + std::to_string(i) + "].aLinear", &m_PointLights[i].aLinear);
			lights->SetValue("pointLights[" + std::to_string(i) + "].aQuadratic", &m_PointLights[i].aQuadratic);
		}

		// Spot Lights
		for (int i = 0; i < num_spots; ++i)
		{
			float angle = m_SpotLights[i].GetAngle();

			lights->SetValue("spotLights[" + std::to_string(i) + "].position", &m_SpotLights[i].position);
			lights->SetValue("spotLights[" + std::to_string(i) + "].direction", &m_SpotLights[i].direction);
			lights->SetValue("spotLights[" + std::to_string(i) + "].intensity", &m_SpotLights[i].intensity);
			lights->SetValue("spotLights[" + std::to_string(i) + "].coneAngle", &angle);
			lights->SetValue("spotLights[" + std::to_string(i) + "].aConstant", &m_SpotLights[i].aConstant);
			lights->SetValue("spotLights[" + std::to_string(i) + "].aLinear", &m_SpotLights[i].aLinear);
			lights->SetValue("spotLights[" + std::to_string(i) + "].aQuadratic", &m_SpotLights[i].aQuadratic);
			lights->SetValue("spotLights[" + std::to_string(i) + "].switched_on", &m_SpotLights[i].switched_on);
		}

		lights->Bind();
	}

	//--------------------------------------------------------------------------------

	ShaderProgram* dsp = m_ResManager->GetShader(STD_DEF_DIR_LIGHT_SHADER);
	dsp->Use();
	dsp->SetUniformValue<Vec3>("u_DirectionalLight.Base.Color", &m_DirLight.intensity);
	dsp->SetUniformValue<Vec3>("u_DirectionalLight.Direction", &m_DirLight.direction);
	dsp->SetUniformValue<float>("u_DirectionalLight.Base.AmbientIntensity", &m_DirLight.ambient_intensity);
	dsp->SetUniformValue<float>("u_DirectionalLight.Base.DiffuseIntensity", &m_DirLight.ambient_intensity);


	// Set for terrain in forward mode
	if (!m_DeferredRender)
	{
		ShaderProgram* terrainShader = m_ResManager->GetShader(TERRAIN_SHADER);
		terrainShader->Use();

		// Dir light
		terrainShader->SetUniformValue<Vec3>("u_DirectionalLight.color", &m_DirLight.intensity);
		terrainShader->SetUniformValue<Vec3>("u_DirectionalLight.dir", &m_DirLight.direction);
		terrainShader->SetUniformValue<float>("u_DirectionalLight.ambient_intensity", &m_DirLight.ambient_intensity);

		for (int i = 0; i < 5; ++i)
		{
			int val = GL_TEXTURE0 + i;
			terrainShader->SetUniformValue<int>("u_Sampler"+std::to_string(i), &val);
		}
	}

	return true;
}

float Renderer::getFrameTime(TimeMeasure tm)
{
	switch (tm)
	{
	case TimeMeasure::NanoSeconds:
		return (float)m_QueryTime;
	case TimeMeasure::MilliSeconds:
		return m_QueryTime / 1000000.f;
	case TimeMeasure::Seconds:
		return m_QueryTime / 100'000'000'0.f;
	}

	return 0.0f;
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