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

Technique::Technique() :
	m_shaderProg(0)
{
}

Technique::~Technique()
{

}

void Technique::Close()
{
	// Delete the intermediate shader objects that have been added to the program
	// The list will only contain something if shaders were compiled but the object itself
	// was destroyed prior to linking.
	for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++)
	{
		OpenGLLayer::clean_GL_shader(&(*it));
	}

	OpenGLLayer::clean_GL_program(&m_shaderProg);
}

bool Technique::Init()
{
	m_shaderProg = glCreateProgram();

	if (m_shaderProg == 0)
	{
		WRITE_LOG("invalid shader prog", "error");
		return false;
	}

	return true;
}

bool Technique::AddShader(GLenum ShaderType, const char* pFilename)
{
	std::string s;

	TextFile tf;
	s = tf.LoadFileIntoStr(std::string(pFilename));
	if (s == "")
	{
		WRITE_LOG("could not load: " + std::string(pFilename), "error");
		return false;
	}

	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0)
	{
		WRITE_LOG("Error creating shader", "error");
		return false;
	}

	// Save the shader object - will be deleted in the destructor
	m_shaderObjList.push_back(ShaderObj);

	const GLchar* p[1];
	p[0] = s.c_str();
	GLint Lengths[1] = { (GLint)s.size() };

	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);

	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		WRITE_LOG(std::string(InfoLog), "error");
		fprintf(stderr, "Error compiling '%s': '%s'\n", pFilename, InfoLog);
		return false;
	}

	glAttachShader(m_shaderProg, ShaderObj);

	return true;
}

bool Technique::Finalize()
{
	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	// NO GOOD HERE
	glBindAttribLocation(m_shaderProg, 0, "Position");
	glBindAttribLocation(m_shaderProg, 1, "Normal");
	glBindAttribLocation(m_shaderProg, 2, "Texcoord");
	glBindFragDataLocation(m_shaderProg, 0, "FragColor");

	glLinkProgram(m_shaderProg);

	glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &Success);
	if (Success == 0)
	{
		glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
		WRITE_LOG("Error linking prog: " + std::string(ErrorLog), "error");
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		return false;
	}

	glValidateProgram(m_shaderProg);
	glGetProgramiv(m_shaderProg, GL_VALIDATE_STATUS, &Success);
	if (!Success)
	{
		glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
		WRITE_LOG("Invalid shader prog: " + std::string(ErrorLog), "error");
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		//   return false;
	}

	// Delete the intermediate shader objects that have been added to the program
	for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++)
	{
		glDeleteShader(*it);
	}

	m_shaderObjList.clear();

	return true;// OpenGLLayer::check_GL_error();
}

void Technique::Enable()
{
	glUseProgram(m_shaderProg);
}

GLint Technique::GetUniformLocation(const char* pUniformName)
{
	GLuint Location = glGetUniformLocation(m_shaderProg, pUniformName);

	if (Location == INVALID_UNIFORM_LOCATION)
	{
		fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
	}

	return Location;
}

GLint Technique::GetProgramParam(GLint param)
{
	GLint ret;
	glGetProgramiv(m_shaderProg, param, &ret);
	return ret;
}


LightingTechnique::LightingTechnique()
{
}

bool LightingTechnique::Init()
{
	if (!Technique::Init()) {
		return false;
	}

	if (!AddShader(GL_VERTEX_SHADER, "../resources/shaders/lighting.vs")) {
		return false;
	}

	if (!AddShader(GL_FRAGMENT_SHADER, "../resources/shaders/lighting.fs")) {
		return false;
	}

	if (!Finalize()) {
		return false;
	}

	m_WVPLocation = GetUniformLocation("gWVP");
	m_WorldMatrixLocation = GetUniformLocation("gWorld");
	m_samplerLocation = GetUniformLocation("gSampler");
	m_eyeWorldPosLocation = GetUniformLocation("gEyeWorldPos");
	m_dirLightLocation.Color = GetUniformLocation("gDirectionalLight.Base.Color");
	m_dirLightLocation.AmbientIntensity = GetUniformLocation("gDirectionalLight.Base.AmbientIntensity");
	m_dirLightLocation.Direction = GetUniformLocation("gDirectionalLight.Direction");
	m_dirLightLocation.DiffuseIntensity = GetUniformLocation("gDirectionalLight.Base.DiffuseIntensity");
	m_matSpecularIntensityLocation = GetUniformLocation("gMatSpecularIntensity");
	m_matSpecularPowerLocation = GetUniformLocation("gSpecularPower");
	
	bool usePoints = true;
	bool useSpots = false;
	
	if (usePoints)
	{
		m_numPointLightsLocation = GetUniformLocation("gNumPointLights");

		for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLightsLocation); i++) {
			char Name[128];
			memset(Name, 0, sizeof(Name));
			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
			m_pointLightsLocation[i].Color = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
			m_pointLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Position", i);
			m_pointLightsLocation[i].Position = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
			m_pointLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
			m_pointLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
			m_pointLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
			m_pointLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

			if (m_pointLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
				m_pointLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
				m_pointLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
				m_pointLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
				m_pointLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
				m_pointLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
				m_pointLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
				return false;
			}
		}
	}

	if (useSpots)
	{
		m_numSpotLightsLocation = GetUniformLocation("gNumSpotLights");

		for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_spotLightsLocation); i++) {
			char Name[128];
			memset(Name, 0, sizeof(Name));
			SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Base.Color", i);
			m_spotLightsLocation[i].Color = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
			m_spotLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Position", i);
			m_spotLightsLocation[i].Position = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Direction", i);
			m_spotLightsLocation[i].Direction = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Cutoff", i);
			m_spotLightsLocation[i].Cutoff = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
			m_spotLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Constant", i);
			m_spotLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Linear", i);
			m_spotLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

			SNPRINTF(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Exp", i);
			m_spotLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

			if (m_spotLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
				m_spotLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
				m_spotLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
				m_spotLightsLocation[i].Direction == INVALID_UNIFORM_LOCATION ||
				m_spotLightsLocation[i].Cutoff == INVALID_UNIFORM_LOCATION ||
				m_spotLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
				m_spotLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
				m_spotLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
				m_spotLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION) {
				return false;
			}
		}
	}

	if (m_dirLightLocation.AmbientIntensity == INVALID_UNIFORM_LOCATION ||
		m_WVPLocation == INVALID_UNIFORM_LOCATION ||
		m_WorldMatrixLocation == INVALID_UNIFORM_LOCATION ||
		m_samplerLocation == INVALID_UNIFORM_LOCATION ||
		m_eyeWorldPosLocation == INVALID_UNIFORM_LOCATION ||
		m_dirLightLocation.Color == INVALID_UNIFORM_LOCATION ||
		m_dirLightLocation.DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
		m_dirLightLocation.Direction == INVALID_UNIFORM_LOCATION ||
		m_matSpecularIntensityLocation == INVALID_UNIFORM_LOCATION ||
		m_matSpecularPowerLocation == INVALID_UNIFORM_LOCATION )
	{
		return false;
	}

	return true;
}

void LightingTechnique::SetWVP(const Mat4& WVP)
{
	glUniformMatrix4fv(m_WVPLocation, 1, GL_FALSE, glm::value_ptr(WVP));
}

void LightingTechnique::SetWorldMatrix(const Mat4& WorldInverse)
{
	glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_FALSE, glm::value_ptr(WorldInverse));
}

void LightingTechnique::SetTextureUnit(unsigned int TextureUnit)
{
	glUniform1i(m_samplerLocation, TextureUnit);
}

void LightingTechnique::SetDirectionalLight(const DirectionalLight& Light)
{
	glUniform3f(m_dirLightLocation.Color, Light.Color.x, Light.Color.y, Light.Color.z);
	glUniform1f(m_dirLightLocation.AmbientIntensity, Light.AmbientIntensity);
	Vec3 Direction = glm::normalize(Light.Direction);
	glUniform3f(m_dirLightLocation.Direction, Direction.x, Direction.y, Direction.z);
	glUniform1f(m_dirLightLocation.DiffuseIntensity, Light.DiffuseIntensity);
}

void LightingTechnique::SetEyeWorldPos(const Vec3& EyeWorldPos)
{
	glUniform3f(m_eyeWorldPosLocation, EyeWorldPos.x, EyeWorldPos.y, EyeWorldPos.z);
}

void LightingTechnique::SetMatSpecularIntensity(float Intensity)
{
	glUniform1f(m_matSpecularIntensityLocation, Intensity);
}

void LightingTechnique::SetMatSpecularPower(float Power)
{
	glUniform1f(m_matSpecularPowerLocation, Power);
}

void LightingTechnique::SetPointLights(unsigned int NumLights, const PointLight* pLights)
{
	glUniform1i(m_numPointLightsLocation, NumLights);

	for (unsigned int i = 0; i < NumLights; i++) {
		glUniform3f(m_pointLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
		glUniform1f(m_pointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
		glUniform1f(m_pointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
		glUniform3f(m_pointLightsLocation[i].Position, pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
		glUniform1f(m_pointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
		glUniform1f(m_pointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
		glUniform1f(m_pointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
	}
}

void LightingTechnique::SetSpotLights(unsigned int NumLights, const SpotLight* pLights)
{
	glUniform1i(m_numSpotLightsLocation, NumLights);

	for (unsigned int i = 0; i < NumLights; i++) {
		glUniform3f(m_spotLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
		glUniform1f(m_spotLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
		glUniform1f(m_spotLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
		glUniform3f(m_spotLightsLocation[i].Position, pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
		Vec3 Direction = glm::normalize(pLights[i].Direction);

		glUniform3f(m_spotLightsLocation[i].Direction, Direction.x, Direction.y, Direction.z);
		glUniform1f(m_spotLightsLocation[i].Cutoff, cosf( Maths::ToRadians(pLights[i].Cutoff)));
		glUniform1f(m_spotLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
		glUniform1f(m_spotLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
		glUniform1f(m_spotLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
	}
}


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

	ShaderAttrib vert_pos, vert_norm, vert_tex;
	vert_pos.layout_location = 0;
	vert_norm.layout_location = 1;
	vert_tex.layout_location = 2;
	vert_pos.name = "vertex_position";
	vert_norm.name = "vertex_normal";
	vert_tex.name = "vertex_texcoord";

	if (!vert.LoadShader("../resources/shaders/lighting_vs.glsl"))
	{
		return false;
	}

	vert.AddAttribute(vert_pos);
	vert.AddAttribute(vert_norm);
	vert.AddAttribute(vert_tex);

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

	if (!GetLocation(m_Ufm_Sampler, "u_Sampler")) { return false; }
	if (!GetLocation(m_Ufm_ShadowSampler, "u_ShadowSampler")) { return false; }

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
	this->setInt_(&m_Ufm_Sampler, textureUnit);
}

void LightTechnique::setShadowSampler(unsigned int sampler)
{
	this->setInt_(&m_Ufm_ShadowSampler, sampler);
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

		if (subMesh.TextureIndex != INVALID_TEXTURE_LOCATION)
		{
			Texture* t = r->GetTexture(m->m_TextureHandles[subMesh.TextureIndex]);

			if (t)
				t->Bind();
			
			//m_Textures[thisMesh->m_TextureHandles[subMesh.TextureIndex]]->Bind();
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
