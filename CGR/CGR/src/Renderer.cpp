#include "Renderer.h"

// Other Graphics
#include "Screen.h"
#include "Mesh.h"
#include "Material.h"
#include "ShadowFrameBuffer.h"
#include "BillboardList.h"
#include "Terrain.h"
#include "Font.h"
#include "Texture.h"
#include "GBuffer.h"
#include "ShaderProgram.h"
#include "ResourceManager.h"
#include "UniformBlockManager.h"
#include "UniformBlock.h"
#include "Lights.h"
#include "Frustum.h"

// Utils
#include "LogFile.h"
#include "utils.h"
#include "EventManager.h"

// Game Object and Components
#include "GameObject.h"
#include "Transform.h"
#include "Camera.h"
#include "MeshRenderer.h"

#include "Input.h"
#include "ResId.h"

// ---- Globals ----
const Mat4 IDENTITY(1.0f);
const bool DO_SHADOWS = true;

Renderer::Renderer() :
	m_ResManager(nullptr),
	m_CameraPtr(nullptr),
	m_Query(),
	m_QueryTime(0),
	m_Gbuffer(nullptr),
	m_Frustum(nullptr),
	m_UniformBlockManager(nullptr),
	m_NumDirLightsInScene(-1),
	m_NumPointLightsInScene(-1),
	m_NumSpotLightsInScene(-1)
{
}

bool Renderer::Init()
{
#ifdef _DEBUG
	m_ShouldQueryFrames = true;
#endif // _DEBUG

	// Log Info
	{
		const GLubyte* rendererS = glGetString(GL_RENDERER);
		const GLubyte* vendorS = glGetString(GL_VENDOR);
		const GLubyte* versionS = glGetString(GL_VERSION);
		std::stringstream ss;
		ss << "Renderer started..... system using: " << rendererS << ",  " << vendorS << ", GL Version: " << versionS;
		WRITE_LOG(ss.str(), "info");

		std::stringstream sss;
		sss << rendererS << " : " << vendorS << " : " << versionS;
		m_HardwareStr = sss.str();
	}

	// Attach events
	EventManager* ev = EventManager::Instance();
	if (ev)
	{
		ev->AttachEvent(EVENT_SCENE_CHANGE, *this);
		ev->AttachEvent(EVENT_WINDOW_SIZE_CHANGE, *this);
	}

	// This is minimal point light info needed for deferred lighting
	m_PointsInfo.resize(MAX_POINTS);

	if(!m_ResManager)
		m_ResManager = new ResourceManager();
	
	m_Query.Init(GL_TIME_ELAPSED);
	
	bool success = true;

	// Create Default Engine uniform blocks, needs to be first
	success &= createUniformBlocks();

	// Create all default engine resources - This needs to be moved as it's a user defined thing 
	success &= m_ResManager->CreateDefaultResources();

	// G Buffer for deferred 
	success &= setFrameBuffers();

	// Set static shader values on default engine shaders
	success &= setStaticDefaultShaderValues();

	if(!m_Frustum)
		m_Frustum = new Frustum();

	return success;
}

void Renderer::Close()
{
	// Attach events
	EventManager* ev = EventManager::Instance();
	if (ev)
	{
		ev->RemoveEvent(EVENT_SCENE_CHANGE, *this);
		ev->RemoveEvent(EVENT_WINDOW_SIZE_CHANGE, *this);
	}

	m_Query.Clean();

	SAFE_DELETE(m_Gbuffer);
	SAFE_DELETE(m_Frustum);
	SAFE_CLOSE(m_UniformBlockManager);
	SAFE_CLOSE(m_ResManager);
}

bool Renderer::SetSceneData(BaseCamera* camera, const Vec3& ambient_light)
{
	// This needs to be called by each scene on load
	m_CameraPtr = camera;

	UniformBlock* scene = m_UniformBlockManager->GetBlock("scene");
	scene->SetValue("ambient_light", (void*)glm::value_ptr(ambient_light));
	return true;
}

const std::string& Renderer::GetHardwareStr() const
{
	return m_HardwareStr;
}

void Renderer::Render(std::vector<GameObject*>& gameObjects)
{
	m_CullCount = 0;

	if(m_ShouldQueryFrames)
		m_Query.Start();

	// Update Uniform blocks for wither rendering mode
	UniformBlock* scene = m_UniformBlockManager->GetBlock("scene");
	if (scene)
	{
		scene->SetValue("view_xform", (void*)glm::value_ptr(m_CameraPtr->View()));
		scene->SetValue("proj_xform", (void*)glm::value_ptr(m_CameraPtr->Projection()));
	}

	// Bind all Uniform Blocks (including custom usr ones)
	for (auto i = m_UniformBlockManager->m_Blocks.begin(); i != m_UniformBlockManager->m_Blocks.end(); ++i)
	{
		if (i->second->ShouldUpdateGPU())
			i->second->Bind();
	}

	if (m_ShadingMode == ShadingMode::Deferred)
		deferredRender(gameObjects);
	else
		forwardRender(gameObjects);

	if (m_ShouldQueryFrames)
	{
		m_Query.End();
		m_QueryTime = m_Query.Result(false);
	}

	// Wait until everything is drawn first
	if (m_ShadingModePending)
	{
		glFlush();

		if (m_ShadingMode == ShadingMode::Deferred)
		{
			glDepthMask(GL_TRUE);
		}

		m_ShadingMode = m_PendingShadingMode;
		m_ShadingModePending = false;
	}

	// Render info if asked
	if (m_ShouldDisplayInfo)
	{
		this->RenderText(FONT_COURIER, "Frm Time Seconds: " + util::to_str(getFrameTime(TimeMeasure::Seconds)), 8, Screen::Instance()->FrameBufferHeight() - 32.0f, FontAlign::Left, Colour::Red());
		this->RenderText(FONT_COURIER, "Frustum cull set to: " + util::bool_to_str(m_ShouldFrustumCull) + " :  Cull count: " + util::to_str(m_CullCount), 8, Screen::Instance()->FrameBufferHeight() - 64.0f);
	}
}

void Renderer::RenderText(size_t fontId, const std::string& txt, float x, float y, FontAlign fa, const Colour& colour)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Activate corresponding render state	
	m_ResManager->m_Shaders[SHADER_FONT_FWD]->Use();

	Mat4 projection = glm::ortho(0.0f, (float)Screen::Instance()->FrameBufferWidth(),
		0.0f,
		(float)Screen::Instance()->FrameBufferHeight());

	m_ResManager->m_Shaders[SHADER_FONT_FWD]->SetUniformValue<Mat4>("u_proj_xform", &projection);
	m_ResManager->m_Shaders[SHADER_FONT_FWD]->SetUniformValue<Vec4>("text_colour", &colour.Normalize());

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

void Renderer::RenderBillboardList(BillboardList* billboard)
{
	if (billboard && m_CameraPtr)
	{
		if (billboard->m_Material)
		{
			billboard->m_Material->Use();
			billboard->m_Material->SetUniformValue<float>("u_Scale", &billboard->m_BillboardScale);
			billboard->m_Material->SetUniformValue<Vec3>("u_camera_position", &m_CameraPtr->Position());
			billboard->m_Material->SetUniformValue<Mat4>("u_view_proj_xform", &m_CameraPtr->ProjXView());
		}

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Bind the texture we get from renderer. This needs sorting too
		Texture* t = this->GetTexture(billboard->m_TextureIndex);
		if (t) t->Bind();

		// I think we shoulf be asking renderer to do this 
		glBindVertexArray(billboard->m_VAO);
		glDrawArrays(GL_POINTS, 0, (GLsizei)billboard->m_NumInstances);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}
}

void Renderer::RenderSurface(SurfaceMesh* surface, const Vec3& position)
{
	if (surface)
	{
		if (surface->m_Material)
		{
			surface->m_Material->Use();
			
			// Bind Materials
			if (m_ResManager->MaterialSetExists(surface->m_MaterialId))
			{
				auto& materials = m_ResManager->m_Materials[surface->m_MaterialId];

				for (auto i = materials.begin(); i != materials.end(); ++i)
				{
					i->second->Bind();
				}
			}

			int n = 0;
			surface->m_Material->SetUniformValue<Mat4>("u_world_xform", &(glm::translate(IDENTITY, position)));
			surface->m_Material->SetUniformValue<int>("u_use_bumpmap", &n);
			surface->m_Material->SetUniformValue<float>("u_MaxHeight", &surface->m_MaxHeight);
			surface->m_Material->SetUniformValue<float>("u_MaxTexU", &surface->m_TexU);
			surface->m_Material->SetUniformValue<float>("u_MaxTexV", &surface->m_TexV);

			glBindVertexArray(surface->m_VAO);
			glDrawElements(GL_TRIANGLES, surface->m_NumIndices, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}
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

int Renderer::GetDirLightIndex()
{
	int temp = m_NumDirLightsInScene + 1;
	return temp < 2 ? ++m_NumDirLightsInScene : -1;
}

int Renderer::GetSpotLightIndex()
{
	int temp = m_NumSpotLightsInScene + 1;
	return temp < MAX_SPOTS ? ++m_NumSpotLightsInScene : -1;
}

int Renderer::GetPointLightIndex()
{
	int temp = m_NumPointLightsInScene + 1;
	if (temp < MAX_POINTS)
	{
		return ++m_NumPointLightsInScene;
	}

	return -1;
	//return temp < MAX_POINTS ? ++m_NumPointLightsInScene : -1;
}

void Renderer::UpdatePointLight(int index, const Vec3& position, float range)
{
	if (index >= 0 && index < MAX_POINTS)
	{
		m_PointsInfo[index].pos = position;
		m_PointsInfo[index].range = range;
	}
}

void Renderer::ToggleShadingMode()
{
	if (m_ShadingMode == ShadingMode::Deferred)
	{
		m_PendingShadingMode = ShadingMode::Forward;
	}
	else
	{
		m_PendingShadingMode = ShadingMode::Deferred;
	}

	m_ShadingModePending = true;
}

void Renderer::SetShadingMode(ShadingMode mode)
{
	m_PendingShadingMode = mode;
	m_ShadingModePending = true;
}

ShadingMode Renderer::GetShadingMode() const
{
	return m_ShadingMode;
}

std::string Renderer::GetShadingModeStr() const
{
	return m_ShadingMode == ShadingMode::Forward ? "Shading mode: Forward" : "Shading mode: Deferred";
}

std::string Renderer::GetPolygonModeStr() const
{
	return m_PolyMode == PolygonMode::Filled? "Polygon mode: Filled" : "Polygon mode: Wireframe";
}

void Renderer::SetPolygonMode(PolygonMode mode)
{
	if (mode == PolygonMode::Filled)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else if (mode == PolygonMode::WireFrame)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
}

void Renderer::DisplayNormals(bool shouldDisplay)
{
	m_ShouldDisplayNormals = shouldDisplay;
}

bool Renderer::IsDisplayingNormals() const
{
	return m_ShouldDisplayNormals;
}

bool Renderer::IsQueeryingFrames() const
{
	return m_ShouldQueryFrames;
}

void Renderer::ToggleFrameQueeryMode()
{
	m_ShouldQueryFrames = !m_ShouldQueryFrames;
}

void Renderer::ToggleFrustumCulling()
{
	m_ShouldFrustumCull = !m_ShouldFrustumCull;
}

void Renderer::SetFrustumCulling(bool should)
{
	m_ShouldFrustumCull = should;
}

void Renderer::ToggleDisplayInfo()
{
	m_ShouldDisplayInfo = !m_ShouldDisplayInfo;
}

void Renderer::SetDisplayInfo(bool should)
{
	m_ShouldDisplayInfo = should;
}


void Renderer::forwardRender(std::vector<GameObject*>& gameObjects)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!m_CameraPtr)
		// Should log if that hasn't been set
		return;

	// Render Skybox
	if (m_CameraPtr->HasSkybox())
	{
		this->renderSkybox(m_CameraPtr);
	}

	m_ResManager->GetShader(SHADER_FRUSTUM)->Use();

	m_ResManager->GetShader(SHADER_FRUSTUM)->SetUniformValue<Mat4>("u_wvp_xform", &(m_CameraPtr->ProjXView() * IDENTITY));
	m_Frustum->UpdateFrustum(m_CameraPtr->Projection(), m_CameraPtr->View());

	// TODO : need to use the shader program that was set to each mesh or batch them 
	ShaderProgram* sp = m_ResManager->m_Shaders[SHADER_LIGHTING_FWD]; //m_Shaders[mr->m_ShaderIndex];
	ShaderProgram* np = nullptr;	
	if(m_ShouldDisplayNormals)
		np = m_ResManager->m_Shaders[SHADER_NORMAL_DISP_FWD];

	// Render Mesh Renderers
	for (auto i = gameObjects.begin(); i != gameObjects.end(); ++i)
	{
		Transform* t = (*i)->GetComponent<Transform>();
		MeshRenderer* mr = (*i)->GetComponent<MeshRenderer>();

		if (!t || !mr)
			continue;

		const Mat4& model_xform = t->GetModelXform();

		if (sp)
		{
			// Render Mesh normally
			sp->Use();
			sp->SetUniformValue<Mat4>("u_world_xform", &(model_xform));
			sp->SetUniformValue<int>("u_use_bumpmap", &(mr->m_HasBumpMaps));

			this->renderMesh(mr, model_xform);

			// Do a normal pass if required
			if (m_ShouldDisplayNormals)
			{
				np->Use();
				np->SetUniformValue<Mat4>("u_wvp", &(m_CameraPtr->ProjXView() * model_xform));
				np->SetUniformValue<Mat4>("u_world_xform", &(model_xform));
				this->renderMesh(mr, IDENTITY, GL_POINTS);
			}
		}
	}
}

void Renderer::deferredRender(std::vector<GameObject*>& gameObjects)
{
	Vec2 screenSize = Vec2((float)Screen::Instance()->FrameBufferWidth(), (float)Screen::Instance()->FrameBufferHeight());
	m_Gbuffer->StartFrame();

	// Geom Pass
	{
		// Skybox
		if (m_CameraPtr->HasSkybox())
			renderSkybox(m_CameraPtr);

		// -- Deferred Geom pass ---
		m_Gbuffer->BindForGeomPass();

		// Set GL States
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ShaderProgram* sp = m_ResManager->m_Shaders[SHADER_GEOM_PASS_DEF];

		ShaderProgram* np = nullptr;
		if (m_ShouldDisplayNormals)
			np = m_ResManager->m_Shaders[SHADER_NORMAL_DISP_FWD];

		// Render Mesh Renderers
		for (auto i = gameObjects.begin(); i != gameObjects.end(); ++i)
		{
			Transform* t = (*i)->GetComponent<Transform>();
			MeshRenderer* mr = (*i)->GetComponent<MeshRenderer>();

			if (!t || !mr)
				continue;

			const Mat4& model_xform = t->GetModelXform();

			if (sp)
			{
				// Render Mesh here
				sp->Use();
				sp->SetUniformValue<Mat4>("u_world_xform", &(model_xform));
				this->renderMesh(mr, model_xform);
			}

			// Do a normal pass if required
			if (m_ShouldDisplayNormals && np)
			{
				np->Use();
				np->SetUniformValue<Mat4>("u_wvp", &(m_CameraPtr->ProjXView() * model_xform));
				np->SetUniformValue<Mat4>("u_world_xform", &(model_xform));
				this->renderMesh(mr, model_xform, GL_POINTS);
			}
		}

		glDepthMask(GL_FALSE);
	}

	// Stencil and Point Lights
	{
		glEnable(GL_STENCIL_TEST);

		for (int i = 0; i < m_NumPointLightsInScene + 1; ++i)
		{
			Mat4 LIGHT_TRANS = glm::translate(Mat4(1.0f), m_PointsInfo[i].pos) * glm::scale(Mat4(1.0f), Vec3(m_PointsInfo[i].range));

			// Stencil
			{
				m_ResManager->m_Shaders[SHADER_STENCIL_PASS_DEF]->Use();

				// Disable color/depth write and enable stencil
				m_Gbuffer->BindForStencilPass();
				glEnable(GL_DEPTH_TEST);

				glDisable(GL_CULL_FACE);

				glClear(GL_STENCIL_BUFFER_BIT);

				// We need the stencil test to be enabled but we want it to succeed always. Only the depth test matters.
				glStencilFunc(GL_ALWAYS, 0, 0);

				glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
				glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

				m_ResManager->m_Shaders[SHADER_STENCIL_PASS_DEF]->SetUniformValue<Mat4>("u_WVP", &(m_CameraPtr->Projection() * m_CameraPtr->View() * LIGHT_TRANS));
				this->renderMesh(m_ResManager->m_Meshes[MESH_ID_SPHERE]);
			}

			// Point Light
			{
				m_Gbuffer->BindForLightPass();

				ShaderProgram* sp = m_ResManager->m_Shaders[SHADER_POINT_LIGHT_PASS_DEF];
				sp->Use();

				sp->SetUniformValue<int>("u_LightIndex", &i);
				sp->SetUniformValue<Vec2>("u_ScreenSize", &screenSize);
				sp->SetUniformValue<Mat4>("u_WVP", &(m_CameraPtr->Projection() * m_CameraPtr->View() * LIGHT_TRANS));

				glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

				glDisable(GL_DEPTH_TEST);
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_ONE, GL_ONE);

				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);

				// Set PointLight
				this->renderMesh(m_ResManager->m_Meshes[MESH_ID_SPHERE]);
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
		m_ResManager->m_Shaders[SHADER_DIR_LIGHT_PASS_DEF]->Use();
		
		// Should only set once
		m_ResManager->m_Shaders[SHADER_DIR_LIGHT_PASS_DEF]->SetUniformValue<Mat4>("u_WVP", &(Mat4(1.0f)));
		m_ResManager->m_Shaders[SHADER_DIR_LIGHT_PASS_DEF]->SetUniformValue<Vec2>("u_ScreenSize", &screenSize);


		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		this->renderMesh(m_ResManager->m_Meshes[MESH_ID_QUAD]);
		glDisable(GL_BLEND);
	}

	// Final Pass
	{
		m_Gbuffer->BindForFinalPass();

		glBlitFramebuffer(0, 0, (int)screenSize.x, (int)screenSize.y,
			0, 0, (int)screenSize.x, (int)screenSize.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
}

void Renderer::renderMesh(Mesh* thisMesh)
{
	// This is user for rendering meshes for deferred lighting
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

void Renderer::renderMesh(MeshRenderer* meshRenderer, const Mat4& world_xform, GLenum renderMode)
{
	Mesh* thisMesh = m_ResManager->m_Meshes[meshRenderer->MeshIndex];
	if (!thisMesh)
		return;

	glBindVertexArray(thisMesh->m_VAO);

	int meshIndex = 0;
	for (std::vector<SubMesh>::iterator j = thisMesh->m_SubMeshes.begin(); j != thisMesh->m_SubMeshes.end(); j++)
	{
		SubMesh subMesh = (*j);

		bool should_render = true;
		if (m_ShouldFrustumCull)
		{
			Vec3 centre = Maths::Vec4To3(world_xform * Vec4(subMesh.centre, 1.0f));
			float r = Maths::Distance(
				Maths::Vec4To3(world_xform * Vec4(subMesh.minvertex, 1.0f)),
				Maths::Vec4To3(world_xform * Vec4(subMesh.maxVertex, 1.0f)));
			should_render = m_Frustum->SphereInFrustum(centre, r);
		}

		if (should_render)
		{
			const size_t	MaterialSet = meshRenderer->m_MaterialIndex;
			const unsigned	MaterialIndex = subMesh.MaterialIndex;

			if (m_ResManager->MaterialSetExists(MaterialSet))
			{
				auto& materials = m_ResManager->m_Materials[MaterialSet];

				if (materials[MaterialIndex])
				{
					materials[MaterialIndex]->Bind();
				}
			}

			/*
			if (thisMesh->HasMaterials())
			{
				const unsigned int MaterialIndex = subMesh.MaterialIndex;
				if(thisMesh->m_Materials[MaterialIndex])
				{
					thisMesh->m_Materials[MaterialIndex]->Bind();
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
			*/

			if (subMesh.NumIndices > 0)
			{
				glDrawElementsBaseVertex(
					renderMode,
					subMesh.NumIndices,
					GL_UNSIGNED_INT,
					(void*)(sizeof(unsigned int) * subMesh.BaseIndex),
					subMesh.BaseVertex);
			}
			else
			{
				glDrawArrays(renderMode, 0, subMesh.NumVertices);
			}
		}
		else
		{
			++m_CullCount;
		}

		++meshIndex;
	}

	glBindVertexArray(0);
}

void Renderer::renderSkybox(BaseCamera* cam)
{
	if(m_ShadingMode == ShadingMode::Deferred)
		glEnable(GL_DEPTH_TEST);
	
	// Use skybox material
	m_ResManager->m_Shaders[SHADER_SKYBOX_ANY]->Use();

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

	m_ResManager->m_Shaders[SHADER_SKYBOX_ANY]->SetUniformValue<Mat4>("world_xform", &(model));

	// Render mesh with texture here, THIS SHOULDNT BE HERE
	glBindVertexArray(m_ResManager->m_Meshes[MESH_ID_CUBE]->m_VAO);
	Texture* t = m_ResManager->m_Textures[sb->textureIndex];
	if (t) t->Bind();

	for (std::vector<SubMesh>::iterator i = m_ResManager->m_Meshes[MESH_ID_CUBE]->m_SubMeshes.begin();
		i != m_ResManager->m_Meshes[MESH_ID_CUBE]->m_SubMeshes.end(); i++)
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

	if(m_ShadingMode == ShadingMode::Deferred)
		glDisable(GL_DEPTH_TEST);
}

void Renderer::HandleEvent(Event* e)
{
	switch (e->GetID())
	{
	case EVENT_SCENE_CHANGE:
		this->sceneChange();
		break;
	case EVENT_WINDOW_SIZE_CHANGE:
		{
			Vec2* params = (Vec2*)e->GetData();
			if (params)
			{
				this->windowSizeChanged((int)params->x, (int)params->y);
			}

			break;
		}
	}
}

void Renderer::sceneChange()
{
	glFlush();

	// Scene Graph will call this when a scene is changed, TODO : Needs to be only callable by that or an event
	m_NumDirLightsInScene = -1;
	m_NumPointLightsInScene = -1;
	m_NumSpotLightsInScene = -1;
}

void Renderer::windowSizeChanged(int w, int h)
{
	// Reset the camera size
	m_CameraPtr->SetAspect(static_cast<float>(w / h));

	// Re init the G Buffer
	if (m_Gbuffer)
	{
		m_Gbuffer->Init();
	}

	// TODO : Anything else that depends on screen size
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

bool Renderer::setStaticDefaultShaderValues()
{
	// Set Light Uniforms
	// Should make these engine constants
	int sampler = 0;
	int normal_sampler = 2;
	ShaderProgram* lsp = m_ResManager->GetShader(SHADER_LIGHTING_FWD);
	if (lsp)
	{
		lsp->Use();
		lsp->SetUniformValue<int>("u_sampler", &sampler);
		lsp->SetUniformValue<int>("u_normal_sampler", &normal_sampler);
	}

	//--------------------------------------------------------------------------------

	return true;
}

bool Renderer::createUniformBlocks()
{
	// This only ever needs to be called once
	if(!m_UniformBlockManager)
		m_UniformBlockManager = new UniformBlockManager();
	
	std::vector<std::string> names;

	//===================   Scene Block  ==================================
	names.push_back("camera_position");
	names.push_back("ambient_light");
	names.push_back("view_xform");
	names.push_back("proj_xform");
	if (!m_UniformBlockManager->CreateBlock("scene", names))
	{
		WRITE_LOG("The renderer can't create a default uniform block with name 'scene' have you used this name for custom block..", "error");
		return false;
	}


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
	{
		WRITE_LOG("The renderer can't create a default uniform block with name 'Lights' have you used this name for custom block..", "error");
		return false;
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
