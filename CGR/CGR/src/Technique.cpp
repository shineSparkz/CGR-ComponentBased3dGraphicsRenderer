#include "Technique.h"
#include "TextFile.h"
#include "LogFile.h"
#include "math_utils.h"
#include "OpenGlLayer.h"

#include "Camera.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Texture.h"
#include <glm\glm.hpp>

#include <sstream>


RenderTechnique::RenderTechnique() :
	m_ShaderProgram(0)
{
}

bool RenderTechnique::CreateProgram(const std::vector<Shader>& shaders, const std::string& fragout_identifier, GLuint frag_loc)
{
	m_ShaderProgram = glCreateProgram();

	std::vector<Shader>::const_iterator it;
	bool has_fragment_shader = false;

	for (it = shaders.begin(); it != shaders.end(); ++it)
	{
		if (it->shader_type == GL_FRAGMENT_SHADER)
			has_fragment_shader = true;

		glAttachShader(m_ShaderProgram, (*it).shader);

		for (size_t j = 0; j < (*it).attributes.size(); ++j)
		{
			glBindAttribLocation(m_ShaderProgram, (*it).attributes[j].layout_location,
				(*it).attributes[j].name.c_str());

			if (OpenGLLayer::check_GL_error())
			{
				WRITE_LOG("Error: binding attrib location in shader", "error");
				return false;
			}
		}
	}

	// Bind Frag
	if (has_fragment_shader)
	{
		glBindFragDataLocation(m_ShaderProgram, frag_loc, fragout_identifier.c_str());
		if (OpenGLLayer::check_GL_error())
		{
			WRITE_LOG("Error: binding frag data location in shader", "error");
			return false;
		}
	}

	glLinkProgram(m_ShaderProgram);

	GLint link_status = 0;
	glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &link_status);

	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(m_ShaderProgram, string_length, NULL, log);
		std::stringstream logger;
		logger << log << std::endl;
		WRITE_LOG(logger.str(), "error");
		return false;
	}

	return true;
}

void RenderTechnique::Use()
{
	glUseProgram(m_ShaderProgram);
}

void RenderTechnique::Close()
{
	OpenGLLayer::clean_GL_program(&m_ShaderProgram);
}

bool RenderTechnique::GetLocation(GLuint& location, const char* data)
{
	location = glGetUniformLocation(m_ShaderProgram, data);

	if (location == INVALID_UNIFORM_LOCATION)
	{
		std::string error = "Invalid uniform location: ";
		error += data;
		WRITE_LOG(error, "error");
		return false;
	}

	return true;
}

void RenderTechnique::setFloat_(GLuint* flt, float val)
{
	glUniform1f(*flt, val);
}

void RenderTechnique::setInt_(GLuint* i, GLint val)
{
	glUniform1i(*i, val);
}

void RenderTechnique::setVec2_(GLuint* i, const Vec2& val)
{
	glUniform2fv(*i, 1, glm::value_ptr(val));
}

void RenderTechnique::setVec3_(GLuint* i, const Vec3& val)
{
	//glUniform3f(*i, val.x, val.y, val.z);
	glUniform3fv(*i, 1, glm::value_ptr(val));
}

void RenderTechnique::setVec4_(GLuint* i, const Vec4& val)
{
	glUniform4fv(*i, 1, glm::value_ptr(val));
}

void RenderTechnique::setMat4_(GLuint* mat, int count, bool transpose, const Mat4& val)
{
	glUniformMatrix4fv(*mat, count, transpose, glm::value_ptr(val));
}


LightTechnique::LightTechnique()
{
}

bool LightTechnique::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	ShaderAttrib vert_pos, vert_norm, vert_tex, vert_tan;
	vert_pos.layout_location = 0;
	vert_norm.layout_location = 1;
	vert_tex.layout_location = 2;
	vert_tan.layout_location = 3;
	
	vert_pos.name = "vertex_position";
	vert_norm.name = "vertex_normal";
	vert_tex.name = "vertex_texcoord";
	vert_tan.name = "vertex_tangent";

	if (!vert.LoadShader("../resources/shaders/lighting_vs.glsl"))
	{
		return false;
	}

	vert.AddAttribute(vert_pos);
	vert.AddAttribute(vert_norm);
	vert.AddAttribute(vert_tex);
	vert.AddAttribute(vert_tan);

	if (!frag.LoadShader("../resources/shaders/lighting_fs.glsl"))
	{
		return false;
	}

	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);

	if (!this->CreateProgram(shaders, "frag_colour", 0))
	{
		WRITE_LOG("Font technique failed to create program", "error");
		return false;
	}

	this->Use();

	// Uniforms - Tedious no?
	if (!GetLocation(m_Ufm_WvpXform, "u_WvpXform")){ return false;}
	if (!GetLocation(m_Ufm_WorldXform, "u_WorldXform")) { return false; }
	if (!GetLocation(m_Ufm_LightWvpXform, "u_LightWvpXform")) { return false;}

	if (!GetLocation(m_Ufm_TextureSampler, "u_Sampler")) { return false; }
	if (!GetLocation(m_Ufm_ShadowSampler, "u_ShadowSampler")) { return false; }
	if (!GetLocation(m_Ufm_NormalSampler, "u_NormalSampler")) { return false; }

	if (!GetLocation(m_Ufm_EyeWorldPos, "u_EyeWorldPos")) { return false; }
	if (!GetLocation(m_Ufm_DirLight.colour, "u_DirectionalLight.Base.Color")) { return false; }
	if (!GetLocation(m_Ufm_DirLight.ambientIntensity, "u_DirectionalLight.Base.AmbientIntensity")) { return false; }
	if (!GetLocation(m_Ufm_DirLight.direction, "u_DirectionalLight.Direction")) { return false; }
	if (!GetLocation(m_Ufm_DirLight.diffuseIntensity, "u_DirectionalLight.Base.DiffuseIntensity")) { return false; }
	if (!GetLocation(m_Ufm_MatSpecularIntensity, "u_MatSpecularIntensity")) { return false; }
	if (!GetLocation(m_Ufm_MatSpecularPower, "u_SpecularPower")) { return false; }
	
	bool usePoints = true;
	bool useSpots = true;
	
	if (usePoints)
	{
		if (!GetLocation(m_Ufm_NumPointLights, "u_NumPointLights")) { return false; }

		for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_Ufm_PointLights); ++i)
		{
			char Name[128];
			memset(Name, 0, sizeof(Name));

			SNPRINTF(Name, sizeof(Name), "u_PointLights[%d].Base.Color", i);
			if (!GetLocation(m_Ufm_PointLights[i].colour, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_PointLights[%d].Base.AmbientIntensity", i);
			if (!GetLocation(m_Ufm_PointLights[i].ambientIntensity, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_PointLights[%d].Position", i);
			if (!GetLocation(m_Ufm_PointLights[i].position, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_PointLights[%d].Base.DiffuseIntensity", i);
			if (!GetLocation(m_Ufm_PointLights[i].diffuseIntensity, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_PointLights[%d].Atten.Constant", i);
			if (!GetLocation(m_Ufm_PointLights[i].Atten.constant, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_PointLights[%d].Atten.Linear", i);
			if (!GetLocation(m_Ufm_PointLights[i].Atten.linear, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_PointLights[%d].Atten.Exp", i);
			if (!GetLocation(m_Ufm_PointLights[i].Atten.exp, Name)) return false;
		}
	}

	if (useSpots)
	{
		if (!GetLocation(m_Ufm_NumSpotLights, "u_NumSpotLights")) { return false; }

		for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_Ufm_SpotLights); i++)
		{
			char Name[128];
			memset(Name, 0, sizeof(Name));

			SNPRINTF(Name, sizeof(Name), "u_SpotLights[%d].Base.Base.Color", i);
			if (!GetLocation(m_Ufm_SpotLights[i].colour, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_SpotLights[%d].Base.Base.AmbientIntensity", i);
			if (!GetLocation(m_Ufm_SpotLights[i].ambientIntensity, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_SpotLights[%d].Base.Position", i);
			if (!GetLocation(m_Ufm_SpotLights[i].position, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_SpotLights[%d].Direction", i);
			if (!GetLocation(m_Ufm_SpotLights[i].direction, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_SpotLights[%d].Cutoff", i);
			if (!GetLocation(m_Ufm_SpotLights[i].cutoff, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_SpotLights[%d].Base.Base.DiffuseIntensity", i);
			if (!GetLocation(m_Ufm_SpotLights[i].diffuseIntensity, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_SpotLights[%d].Base.Atten.Constant", i);
			if (!GetLocation(m_Ufm_SpotLights[i].Atten.constant, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_SpotLights[%d].Base.Atten.Linear", i);
			if (!GetLocation(m_Ufm_SpotLights[i].Atten.linear, Name)) return false;

			SNPRINTF(Name, sizeof(Name), "u_SpotLights[%d].Base.Atten.Exp", i);
			if (!GetLocation(m_Ufm_SpotLights[i].Atten.exp, Name)) return false;
		}
	}

	return true;
}

void LightTechnique::setWVP(const Mat4& WVP)
{
	this->setMat4_(&m_Ufm_WvpXform, 1, GL_FALSE, WVP);
}

void LightTechnique::setWorldMatrix(const Mat4& world)
{
	this->setMat4_(&m_Ufm_WorldXform, 1, GL_FALSE, world);
}

void LightTechnique::setLightWVP(const Mat4& lwvp)
{
	this->setMat4_(&m_Ufm_LightWvpXform, 1, GL_FALSE, lwvp);
}

void LightTechnique::setTextureUnit(unsigned int textureUnit)
{
	this->setInt_(&m_Ufm_TextureSampler, textureUnit);
}

void LightTechnique::setShadowSampler(unsigned int sampler)
{
	this->setInt_(&m_Ufm_ShadowSampler, sampler);
}

void LightTechnique::setNormalSampler(unsigned int sampler)
{
	this->setInt_(&m_Ufm_NormalSampler, sampler);
}

void LightTechnique::setDirectionalLight(const DirectionalLight& light)
{
	this->setVec3_(&m_Ufm_DirLight.colour, light.Color);
	this->setVec3_(&m_Ufm_DirLight.direction, glm::normalize(light.Direction));
	this->setFloat_(&m_Ufm_DirLight.ambientIntensity, light.AmbientIntensity);
	this->setFloat_(&m_Ufm_DirLight.diffuseIntensity, light.DiffuseIntensity);
}

void LightTechnique::setEyeWorldPos(const Vec3& eyeWorldPos)
{
	this->setVec3_(&m_Ufm_EyeWorldPos, eyeWorldPos);
}

void LightTechnique::setMatSpecularIntensity(float intensity)
{
	this->setFloat_(&m_Ufm_MatSpecularIntensity, intensity);
}

void LightTechnique::setMatSpecularPower(float power)
{
	this->setFloat_(&m_Ufm_MatSpecularPower, power);
}

void LightTechnique::setPointLights(unsigned int numLights, const PointLight* pLights)
{
	this->setInt_(&m_Ufm_NumPointLights, numLights);
	
	if (pLights)
	{
		for (unsigned int i = 0; i < numLights; ++i)
		{
			this->setVec3_(&m_Ufm_PointLights[i].colour, pLights[i].Color);
			this->setVec3_(&m_Ufm_PointLights[i].position, pLights[i].Position);

			this->setFloat_(&m_Ufm_PointLights[i].ambientIntensity, pLights[i].AmbientIntensity);
			this->setFloat_(&m_Ufm_PointLights[i].diffuseIntensity, pLights[i].DiffuseIntensity);
			this->setFloat_(&m_Ufm_PointLights[i].Atten.constant, pLights[i].Attenuation.Constant);
			this->setFloat_(&m_Ufm_PointLights[i].Atten.linear, pLights[i].Attenuation.Linear);
			this->setFloat_(&m_Ufm_PointLights[i].Atten.exp, pLights[i].Attenuation.Exp);
		}
	}
}

void LightTechnique::setSpotLights(unsigned int numLights, const SpotLight* pLights)
{
	this->setInt_(&m_Ufm_NumSpotLights, numLights);

	if (pLights)
	{
		for (unsigned int i = 0; i < numLights; ++i)
		{
			this->setVec3_(&m_Ufm_SpotLights[i].colour, pLights[i].Color);
			this->setVec3_(&m_Ufm_SpotLights[i].position, pLights[i].Position);
			this->setVec3_(&m_Ufm_SpotLights[i].direction, glm::normalize(pLights[i].Direction));

			this->setFloat_(&m_Ufm_SpotLights[i].diffuseIntensity, pLights[i].DiffuseIntensity);
			this->setFloat_(&m_Ufm_SpotLights[i].ambientIntensity, pLights[i].AmbientIntensity);
			this->setFloat_(&m_Ufm_SpotLights[i].cutoff, pLights[i].Cutoff);
			this->setFloat_(&m_Ufm_SpotLights[i].Atten.constant, pLights[i].Attenuation.Constant);
			this->setFloat_(&m_Ufm_SpotLights[i].Atten.linear, pLights[i].Attenuation.Linear);
			this->setFloat_(&m_Ufm_SpotLights[i].Atten.exp, pLights[i].Attenuation.Exp);
		}
	}
}


FontTechnique::FontTechnique() :
	m_Ufm_ProjId(0),
	m_Ufm_ColId(0)
{
}

bool FontTechnique::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	ShaderAttrib vert_pos;
	vert_pos.layout_location = 0;
	vert_pos.name = "vertex_position";

	if (!vert.LoadShader("../resources/shaders/font_vs.glsl"))
	{
		return false;
	}
	
	vert.AddAttribute(vert_pos);

	if (!frag.LoadShader("../resources/shaders/font_fs.glsl"))
	{
		return false;
	}

	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);

	if (!this->CreateProgram(shaders,"frag_colour",0))
	{
		WRITE_LOG("Font technique failed to create program", "error");
		return false;
	}

	// Uniforms
	if (!GetLocation(m_Ufm_ProjId, "proj_xform"))
	{
		return false;
	}

	if (!GetLocation(m_Ufm_ColId, "text_colour"))
	{
		return false;
	}

	vert.Close();
	frag.Close();

	return true;
}

void FontTechnique::setColour(const Vec4& col)
{
	setVec4_(&m_Ufm_ColId, col);
}

void FontTechnique::setProjection(const Mat4& proj)
{
	setMat4_(&m_Ufm_ProjId, 1, GL_FALSE, proj);
}


SkyboxTechnique::SkyboxTechnique() :
	m_Ufm_WVP(0),
	m_Ufm_TexLoc(0)
{

}

bool SkyboxTechnique::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	ShaderAttrib vert_pos;
	vert_pos.layout_location = 0;
	vert_pos.name = "vertex_position";

	if (!vert.LoadShader("../resources/shaders/skybox_vs.glsl"))
	{
		return false;
	}

	vert.AddAttribute(vert_pos);

	if (!frag.LoadShader("../resources/shaders/skybox_fs.glsl"))
	{
		return false;
	}

	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);

	if (!this->CreateProgram(shaders, "frag_colour", 0))
	{
		WRITE_LOG("Font technique failed to create program", "error");
		return false;
	}

	// Uniforms
	if (!GetLocation(m_Ufm_WVP, "wvp_xform"))
	{
		return false;
	}

	if (!GetLocation(m_Ufm_TexLoc, "cube_sampler"))
	{
		return false;
	}

	vert.Close();
	frag.Close();
	return true;
}

void SkyboxTechnique::Render(Camera* cam, Mesh* m, Renderer* r)
{
	this->Use();

	GLint oldCullMode, oldDepthFunc;
	glGetIntegerv(GL_CULL_FACE_MODE, &oldCullMode);
	glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);

	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	// Hardcode for now
	Mat4 id(1.0f);
	Mat4 model = glm::translate(id, cam->position) *
		// rot
		glm::scale(id, Vec3(20.0f));

	this->setWVP(cam->projection * cam->view * model);
	//this->setTextureUnit(GL_TEXTURE0);
	this->setTextureUnit(0);

	// Render mesh with texture here, THIS SHOULDNT BE HERE
	glBindVertexArray(m->m_VAO);
	for (std::vector<SubMesh>::iterator i = m->m_MeshLayouts.begin();
		i != m->m_MeshLayouts.end(); i++)
	{
		SubMesh subMesh = (*i);

		for (auto tex = subMesh.TextureIndices.begin(); tex != subMesh.TextureIndices.end(); ++tex)
		{
			Texture* t = r->GetTexture(m->m_TextureHandles[(*tex)]);
			if (t)
				t->Bind();
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

	glCullFace(oldCullMode);
	glDepthFunc(oldDepthFunc);
}

void SkyboxTechnique::setWVP(const Mat4& WVP)
{
	this->setMat4_(&m_Ufm_WVP, 1, GL_FALSE, WVP);
}

void SkyboxTechnique::setTextureUnit(unsigned int textureUnit)
{
	this->setInt_(&m_Ufm_TexLoc, textureUnit);
}


BasicDiffuseTechnique::BasicDiffuseTechnique() :
	m_Ufm_ModelXform(0),
	m_Ufm_ViewXform(0),
	m_Ufm_ProjXform(0)
{
}

bool BasicDiffuseTechnique::Init()
{
	// Create Sdaders
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	// Set Attribute locations
	ShaderAttrib vert_pos;
	vert_pos.layout_location = 0;
	vert_pos.name = "vertex_position";
	ShaderAttrib norm_pos{ 1, "vertex_normal" };
	ShaderAttrib tex_pos{ 2, "vertex_texcoord" };

	// Compile
	if (!vert.LoadShader("../resources/shaders/test_vs.glsl")){ return false;}

	vert.AddAttribute(vert_pos);
	vert.AddAttribute(norm_pos);
	vert.AddAttribute(tex_pos);

	if (!frag.LoadShader("../resources/shaders/test_fs.glsl")) { return false;}

	// Create a program with list of shaders
	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);
	if (!this->CreateProgram(shaders, "frag_colour", 0)){ WRITE_LOG("Diffuse technique failed to create program", "error"); return false;}

	// Set Uniforms
	if (!GetLocation(m_Ufm_ModelXform, "model_xform")){ return false;}
	if (!GetLocation(m_Ufm_ViewXform, "view_xform")){ return false; }
	if (!GetLocation(m_Ufm_ProjXform, "proj_xform")){ return false;}

	// Delete these shaders, no need to hold on to them
	vert.Close();
	frag.Close();

	return true;
}

void BasicDiffuseTechnique::setModelXform(const Mat4& m)
{
	setMat4_(&m_Ufm_ModelXform, 1, GL_FALSE, m);
}

void BasicDiffuseTechnique::setProjXform(const Mat4& m)
{
	setMat4_(&m_Ufm_ProjXform, 1, GL_FALSE, m);
}

void BasicDiffuseTechnique::setViewXform(const Mat4& m)
{
	setMat4_(&m_Ufm_ViewXform, 1, GL_FALSE, m);
}


ShadowMapTechnique::ShadowMapTechnique()
{
}

bool ShadowMapTechnique::Init()
{
	// Create Sdaders
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	// Set Attribute locations
	ShaderAttrib vert_pos;
	vert_pos.layout_location = 0;
	vert_pos.name = "vertex_position";
	ShaderAttrib norm_pos{ 1, "vertex_normal" };
	ShaderAttrib tex_pos{ 2, "vertex_texcoord" };

	// Compile
	if (!vert.LoadShader("../resources/shaders/shadowmap_vs.glsl")) { return false; }

	vert.AddAttribute(vert_pos);
	vert.AddAttribute(norm_pos);
	vert.AddAttribute(tex_pos);

	if (!frag.LoadShader("../resources/shaders/shadowmap_fs.glsl")) { return false; }

	// Create a program with list of shaders
	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);
	if (!this->CreateProgram(shaders, "frag_colour", 0)) { WRITE_LOG("Shadowmap technique failed to create program", "error"); return false; }

	// Set Uniforms
	if (!GetLocation(m_Ufm_WvpXform, "u_wvp_xform")) { return false; }
	if (!GetLocation(m_Ufm_TextureLocation, "u_shadow_sampler")) { return false; }

	// Delete these shaders, no need to hold on to them
	vert.Close();
	frag.Close();

	return true;
}

void ShadowMapTechnique::setWvpXform(const Mat4& WVP)
{
	this->setMat4_(&m_Ufm_WvpXform, 1, GL_FALSE, WVP);
}

void ShadowMapTechnique::setTextureUnit(unsigned int textureUnit)
{
	this->setInt_(&m_Ufm_TextureLocation, textureUnit);
}



BillboardTechnique::BillboardTechnique() :
	m_Ufm_CamPos(0),
	m_Ufm_TextureMap(0),
	m_Ufm_ViewProj(0)
{
}

bool BillboardTechnique::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader geom(GL_GEOMETRY_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	// Set Attribute locations
	ShaderAttrib vert_pos;
	vert_pos.layout_location = 0;
	vert_pos.name = "vertex_position";
	vert.AddAttribute(vert_pos);

	if (!vert.LoadShader("../resources/shaders/billboard_vs.glsl")) { return false; }
	if (!geom.LoadShader("../resources/shaders/billboard_gs.glsl")) { return false; }
	if (!frag.LoadShader("../resources/shaders/billboard_fs.glsl")) { return false; }

	// Create a program with list of shaders
	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(geom);
	shaders.push_back(frag);
	if (!this->CreateProgram(shaders, "frag_colour", 0)) { WRITE_LOG("Billboard technique failed to create program", "error"); return false; }

	// Set Uniforms
	if (!GetLocation(m_Ufm_CamPos, "u_CamPos")) { return false; }
	if (!GetLocation(m_Ufm_Scale, "u_Scale")) { return false; }
	if (!GetLocation(m_Ufm_TextureMap, "u_TextureMap")) { return false; }
	if (!GetLocation(m_Ufm_ViewProj, "u_ViewProjXform")) { return false; }

	// Delete these shaders, no need to hold on to them
	vert.Close();
	geom.Close();
	frag.Close();

	return true;
}

void BillboardTechnique::setViewProjXform(const Mat4& vp)
{
	this->setMat4_(&m_Ufm_ViewProj, 1, false, vp);
}

void BillboardTechnique::setCamPos(const Vec3& pos)
{
	this->setVec3_(&m_Ufm_CamPos, pos);
}

void BillboardTechnique::setTexureMapSampler(unsigned sampler)
{
	this->setInt_(&m_Ufm_TextureMap, sampler);
}

void BillboardTechnique::setBillboardScale(float scale)
{
	this->setFloat_(&m_Ufm_Scale, scale);
}



TerrainTechnique::TerrainTechnique()
{
}

bool TerrainTechnique::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	ShaderAttrib vert_pos, vert_norm, vert_tex;
	vert_pos.layout_location = 0;
	vert_norm.layout_location = 1;
	vert_tex.layout_location = 2;
	vert_pos.name = "vertex_position";
	vert_norm.name = "vertex_normal";
	vert_tex.name = "vertex_texcoord";

	vert.AddAttribute(vert_pos);
	vert.AddAttribute(vert_norm);
	vert.AddAttribute(vert_tex);

	if (!vert.LoadShader("../resources/shaders/terrain_vs.glsl"))
	{
		return false;
	}

	if (!frag.LoadShader("../resources/shaders/terrain_fs.glsl"))
	{
		return false;
	}

	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);

	if (!this->CreateProgram(shaders, "frag_colour", 0))
	{
		WRITE_LOG("Font technique failed to create program", "error");
		return false;
	}

	this->Use();

	// Uniforms - Tedious no?
	if (!GetLocation(m_Ufm_DirLight.colour, "u_DirectionalLight.color")) { return false; }
	if (!GetLocation(m_Ufm_DirLight.ambientIntensity, "u_DirectionalLight.ambientIntensity")) { return false; }
	if (!GetLocation(m_Ufm_DirLight.direction, "u_DirectionalLight.dir")) { return false; }

	if (!GetLocation(m_Ufm_Xforms.proj,  "matrices.projXform")) { return false; }
	if (!GetLocation(m_Ufm_Xforms.view,  "matrices.viewXform")) { return false; }
	if (!GetLocation(m_Ufm_Xforms.model, "matrices.modelXform")) { return false; }

	if (!GetLocation(m_Ufm_MaxTexV, "u_MaxTexV")) { return false; }
	if (!GetLocation(m_Ufm_MaxTexU, "u_MaxTexU")) { return false; }
	if (!GetLocation(m_Ufm_RenderHeight, "u_RenderHeight")) { return false; }
	if (!GetLocation(m_Ufm_Colour, "u_Colour")) { return false; }
	//if (!GetLocation(m_Ufm_ShadowMap, "u_ShadowMap")) { return false; }
	if (!GetLocation(m_Ufm_HeightMapScaleXform, "u_HeightMapScaleXform")) { return false; }

	
	for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_Ufm_Samplers); ++i)
	{
		char Name[128];
		memset(Name, 0, sizeof(Name));

		SNPRINTF(Name, sizeof(Name), "u_Sampler[%d]", i);
		if (!GetLocation(m_Ufm_Samplers[i], Name)) return false;
	}
	
	return true;
}

void TerrainTechnique::setMatrices(Camera* camera, const Mat4& model)
{
	this->setMat4_(&m_Ufm_Xforms.view,  1, false, camera->view);
	this->setMat4_(&m_Ufm_Xforms.proj,  1, false, camera->projection);
	this->setMat4_(&m_Ufm_Xforms.model, 1, false, model);
}

void TerrainTechnique::setHeightMapScaleXform(const Mat4& vp)
{
	this->setMat4_(&m_Ufm_HeightMapScaleXform, 1, false, vp);
}

void TerrainTechnique::setTexSampler(int arrayIndex, int textureIndex)
{
	this->setInt_(&m_Ufm_Samplers[Maths::Clamp(arrayIndex, 0, 4)], textureIndex);
}

void TerrainTechnique::setShadowSampler(int samplerIndex)
{
	//this->setInt_(&m_Ufm_ShadowMap, samplerIndex);
}

void TerrainTechnique::setColour(const Vec4& colour)
{
	this->setVec4_(&m_Ufm_Colour, colour);
}

void TerrainTechnique::setDirectionalLight(const DirectionalLight& light)
{
	this->setVec3_(&m_Ufm_DirLight.colour, light.Color);
	this->setVec3_(&m_Ufm_DirLight.direction, glm::normalize(light.Direction));
	this->setFloat_(&m_Ufm_DirLight.ambientIntensity, light.AmbientIntensity);
}

void TerrainTechnique::setRenderHeight(float val)
{
	this->setFloat_(&m_Ufm_RenderHeight, val);
}

void TerrainTechnique::setMaxTexU(float val)
{
	this->setFloat_(&m_Ufm_MaxTexU, val);
}

void TerrainTechnique::setMaxTexV(float val)
{
	this->setFloat_(&m_Ufm_MaxTexV, val);
}


LavaTechnique::LavaTechnique() :
	m_Ufm_WVP(0),
	m_Ufm_Time(0),
	m_Ufm_TexSampler(0),
	m_Ufm_Resolution(0)
{
}

LavaTechnique::~LavaTechnique()
{
}

bool LavaTechnique::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	ShaderAttrib vert_pos, vert_norm, vert_tex;
	vert_pos.layout_location = 0;
	vert_norm.layout_location = 1;
	vert_tex.layout_location = 2;
	vert_pos.name = "vertex_position";
	vert_norm.name = "vertex_normal";
	vert_tex.name = "vertex_texcoord";
	vert.AddAttribute(vert_pos);
	vert.AddAttribute(vert_norm);
	vert.AddAttribute(vert_tex);

	if (!vert.LoadShader("../resources/shaders/lava_vs.glsl")){ return false; }
	if (!frag.LoadShader("../resources/shaders/lava_fs.glsl")){return false;}

	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);

	if (!this->CreateProgram(shaders, "frag_colour", 0))
	{
		WRITE_LOG("Font technique failed to create program", "error");
		return false;
	}

	this->Use();

	// Uniforms - Tedious no?
	if (!GetLocation(m_Ufm_Time, "u_GlobalTime")) { return false; }
	if (!GetLocation(m_Ufm_WVP, "u_WvpXform")) { return false; }
	if (!GetLocation(m_Ufm_Resolution, "u_Resolution")) { return true; }
	if (!GetLocation(m_Ufm_TexSampler, "u_Sampler")) { return true; }// false;

	return true;
}

void LavaTechnique::setWvpXform(const Mat4& wvp)
{
	this->setMat4_(&m_Ufm_WVP, 1, false, wvp);
}

void LavaTechnique::setTime(float time)
{
	this->setFloat_(&m_Ufm_Time, time);
}

void LavaTechnique::setTexSampler(int sampler)
{
	this->setInt_(&m_Ufm_TexSampler, sampler);
}

void LavaTechnique::setResolution(const Vec2& res)
{
	this->setVec2_(&m_Ufm_Resolution, res);
}


// ---- Deferred ----
GeometryPassTechnique::GeometryPassTechnique() :
	m_Ufm_WVP(0),
	m_Ufm_World(0),
	m_Ufm_ColourSampler(0)
{
}

GeometryPassTechnique::~GeometryPassTechnique()
{
}

bool GeometryPassTechnique::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	ShaderAttrib vert_pos, vert_norm, vert_tex;
	vert_pos.layout_location = 0;
	vert_norm.layout_location = 1;
	vert_tex.layout_location = 2;
	vert_pos.name = "vertex_position";
	vert_norm.name = "vertex_normal";
	vert_tex.name = "vertex_texcoord";
	vert.AddAttribute(vert_pos);
	vert.AddAttribute(vert_norm);
	vert.AddAttribute(vert_tex);

	if (!vert.LoadShader("../resources/shaders/deferred/geometry_pass_vs.glsl")) { return false; }
	if (!frag.LoadShader("../resources/shaders/deferred/geometry_pass_fs.glsl")) { return false; }

	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);

	if (!this->CreateProgram(shaders, "frag_colour", 0))
	{
		WRITE_LOG("Font technique failed to create program", "error");
		return false;
	}

	this->Use();

	if (!GetLocation(m_Ufm_WVP, "u_WVP")) { return false; }
	if (!GetLocation(m_Ufm_World, "u_World")) { return false; }
	if (!GetLocation(m_Ufm_ColourSampler, "u_ColourMap")) { return false; }

	return true;
}

void GeometryPassTechnique::setWVP(const Mat4& wvp)
{
	this->setMat4_(&m_Ufm_WVP, 1, false, wvp);
}

void GeometryPassTechnique::setWorld(const Mat4& w)
{
	this->setMat4_(&m_Ufm_World, 1, false, w);
}

void GeometryPassTechnique::setColourSampler(int sampler)
{
	this->setInt_(&m_Ufm_ColourSampler, sampler);
}


DSLightPassTech::DSLightPassTech() :
	m_WVPLocation(0),
	m_PosTextureUnitLocation(0),
	m_NormalTextureUnitLocation(0),
	m_ColorTextureUnitLocation(0),
	m_EyeWorldPosLocation(0),
	m_MatSpecularIntensityLocation(0),
	m_MatSpecularPowerLocation(0),
	m_ScreenSizeLocation(0)
{
}

bool DSLightPassTech::Init()
{
	if (!this->GetLocation(m_WVPLocation, "u_WVP")) return false;
	if (!this->GetLocation(m_PosTextureUnitLocation, "u_PositionMap")) return false;
	if (!this->GetLocation(m_ColorTextureUnitLocation, "u_ColourMap")) return false;
	if (!this->GetLocation(m_NormalTextureUnitLocation, "u_NormalMap")) return false;
	if (!this->GetLocation(m_EyeWorldPosLocation, "u_EyeWorldPos")) return false;
	if (!this->GetLocation(m_MatSpecularIntensityLocation, "u_MatSpecularIntensity")) return false;
	if (!this->GetLocation(m_MatSpecularPowerLocation, "u_SpecularPower")) return false;
	if (!this->GetLocation(m_ScreenSizeLocation, "u_ScreenSize")) return false;

	return true;
}

void DSLightPassTech::setWVP(const Mat4& WVP)
{
	this->setMat4_(&m_WVPLocation, 1, GL_FALSE, WVP);
}

void DSLightPassTech::setPositionTextureUnit(unsigned int textureUnit)
{
	this->setInt_(&m_PosTextureUnitLocation, textureUnit);
}

void DSLightPassTech::setColorTextureUnit(unsigned int textureUnit)
{
	this->setInt_(&m_ColorTextureUnitLocation, textureUnit);
}

void DSLightPassTech::setNormalTextureUnit(unsigned int textureUnit)
{
	this->setInt_(&m_NormalTextureUnitLocation, textureUnit);
}
	
void DSLightPassTech::setEyeWorldPos(const Vec3& eyeWorldPos)
{
	this->setVec3_(&m_EyeWorldPosLocation, eyeWorldPos);
}

void DSLightPassTech::setMatSpecularIntensity(float intensity)
{
	this->setFloat_(&m_MatSpecularIntensityLocation, intensity);
}

void DSLightPassTech::setMatSpecularPower(float power)
{
	this->setFloat_(&m_MatSpecularPowerLocation, power);
}

void DSLightPassTech::setScreenSize(const Vec2& size)
{
	this->setVec2_(&m_ScreenSizeLocation, size);
}



DSDirLightPassTech::DSDirLightPassTech()
{
}

bool DSDirLightPassTech::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	ShaderAttrib vert_pos, vert_norm, vert_tex;
	vert_pos.layout_location = 0;
	vert_norm.layout_location = 1;
	vert_tex.layout_location = 2;
	vert_pos.name = "vertex_position";
	vert_norm.name = "vertex_normal";
	vert_tex.name = "vertex_texcoord";
	vert.AddAttribute(vert_pos);
	vert.AddAttribute(vert_norm);
	vert.AddAttribute(vert_tex);

	if (!vert.LoadShader("../resources/shaders/deferred/light_pass_vs.glsl")) { return false; }
	if (!frag.LoadShader("../resources/shaders/deferred/dir_light_pass_fs.glsl")) { return false; }

	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);

	if (!this->CreateProgram(shaders, "frag_colour", 0))
	{
		WRITE_LOG("Font technique failed to create program", "error");
		return false;
	}

	this->Use();

	if (!GetLocation(m_DirLightLocation.ambientIntensity, "u_DirectionalLight.Base.AmbientIntensity")) { return false; }
	if (!GetLocation(m_DirLightLocation.colour, "u_DirectionalLight.Base.Color")) { return false; }
	if (!GetLocation(m_DirLightLocation.diffuseIntensity, "u_DirectionalLight.Base.DiffuseIntensity")) { return false; }
	if (!GetLocation(m_DirLightLocation.direction, "u_DirectionalLight.Direction")) { return false; }

	return DSLightPassTech::Init();
}

void DSDirLightPassTech::setDirectionalLight(const DirectionalLight& light)
{
	this->setVec3_(&m_DirLightLocation.colour, light.Color);
	this->setVec3_(&m_DirLightLocation.direction, glm::normalize(light.Direction));
	this->setFloat_(&m_DirLightLocation.ambientIntensity, light.AmbientIntensity);
	this->setFloat_(&m_DirLightLocation.diffuseIntensity, light.DiffuseIntensity);
}


DSPointLightPassTech::DSPointLightPassTech()
{
}

bool DSPointLightPassTech::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	ShaderAttrib vert_pos, vert_norm, vert_tex;
	vert_pos.layout_location = 0;
	vert_norm.layout_location = 1;
	vert_tex.layout_location = 2;
	vert_pos.name = "vertex_position";
	vert_norm.name = "vertex_normal";
	vert_tex.name = "vertex_texcoord";
	vert.AddAttribute(vert_pos);
	vert.AddAttribute(vert_norm);
	vert.AddAttribute(vert_tex);

	if (!vert.LoadShader("../resources/shaders/deferred/light_pass_vs.glsl")) { return false; }
	if (!frag.LoadShader("../resources/shaders/deferred/point_light_pass_fs.glsl")) { return false; }

	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);

	if (!this->CreateProgram(shaders, "frag_colour", 0))
	{
		WRITE_LOG("Font technique failed to create program", "error");
		return false;
	}

	this->Use();

	if (!GetLocation(m_PointLightLocation.colour, "u_PointLight.Base.Color")) { return false; }
	if (!GetLocation(m_PointLightLocation.ambientIntensity, "u_PointLight.Base.AmbientIntensity")) { return false; }
	if (!GetLocation(m_PointLightLocation.position, "u_PointLight.Position")) { return false; }
	if (!GetLocation(m_PointLightLocation.diffuseIntensity, "u_PointLight.Base.DiffuseIntensity")) { return false; }

	if (!GetLocation(m_PointLightLocation.Atten.constant, "u_PointLight.Atten.Constant")) { return false; }
	if (!GetLocation(m_PointLightLocation.Atten.exp, "u_PointLight.Atten.Exp")) { return false; }
	if (!GetLocation(m_PointLightLocation.Atten.linear, "u_PointLight.Atten.Linear")) { return false; }

	return DSLightPassTech::Init();
}

void DSPointLightPassTech::setPointLight(const PointLight& light)
{
	this->setVec3_(&m_PointLightLocation.colour, light.Color);
	this->setVec3_(&m_PointLightLocation.position, light.Position);
	this->setFloat_(&m_PointLightLocation.ambientIntensity, light.AmbientIntensity);
	this->setFloat_(&m_PointLightLocation.diffuseIntensity, light.DiffuseIntensity);
	this->setFloat_(&m_PointLightLocation.Atten.constant, light.Attenuation.Constant);
	this->setFloat_(&m_PointLightLocation.Atten.exp, light.Attenuation.Exp);
	this->setFloat_(&m_PointLightLocation.Atten.linear, light.Attenuation.Linear);
}

NullTechnique::NullTechnique() :
	m_WVPLocation(0)
{
}

bool NullTechnique::Init()
{
	Shader vert(GL_VERTEX_SHADER);
	Shader frag(GL_FRAGMENT_SHADER);

	ShaderAttrib vert_pos;
	vert_pos.layout_location = 0;
	vert_pos.name = "vertex_position";
	vert.AddAttribute(vert_pos);

	if (!vert.LoadShader("../resources/shaders/deferred/stencil_pass_vs.glsl")) { return false; }
	if (!frag.LoadShader("../resources/shaders/deferred/stencil_pass_fs.glsl")) { return false; }

	std::vector<Shader> shaders;
	shaders.push_back(vert);
	shaders.push_back(frag);

	if (!this->CreateProgram(shaders, "frag_colour", 0))
	{
		WRITE_LOG("Font technique failed to create program", "error");
		return false;
	}

	this->Use();

	if (!GetLocation(m_WVPLocation, "u_WVP")) return false;

	return true;
}

void NullTechnique::setWVP(const Mat4& WVP)
{
	this->setMat4_(&m_WVPLocation, 1, GL_FALSE, WVP);
}