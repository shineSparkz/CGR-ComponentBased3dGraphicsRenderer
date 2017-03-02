#ifndef __TECHNIQUE_H__
#define __TECHNIQUE_H__

#include <list>
#include <vector>
#include <string>
#include "gl_headers.h"
#include "types.h"
#include "Lights.h"

#include "Shader.h"

class Technique
{
public:
	Technique();
	virtual ~Technique();

	void Close();

	virtual bool Init();
	void Enable();

protected:
	bool AddShader(GLenum ShaderType, const char* pFilename);
	bool Finalize();

	GLint GetUniformLocation(const char* pUniformName);
	GLint GetProgramParam(GLint param);

	GLuint m_shaderProg;

private:
	typedef std::list<GLuint> ShaderObjList;
	ShaderObjList m_shaderObjList;
};

class LightingTechnique : public Technique
{
public:
	static const unsigned int MAX_POINT_LIGHTS = 2;
	static const unsigned int MAX_SPOT_LIGHTS = 2;

	LightingTechnique();

	virtual bool Init();

	void SetWVP(const Mat4& WVP);
	void SetWorldMatrix(const Mat4& WVP);
	void SetTextureUnit(unsigned int TextureUnit);
	void SetDirectionalLight(const DirectionalLight& Light);
	void SetPointLights(unsigned int NumLights, const PointLight* pLights);
	void SetSpotLights(unsigned int NumLights, const SpotLight* pLights);
	void SetEyeWorldPos(const Vec3& EyeWorldPos);
	void SetMatSpecularIntensity(float Intensity);
	void SetMatSpecularPower(float Power);

private:
	GLuint m_WVPLocation;
	GLuint m_WorldMatrixLocation;
	GLuint m_samplerLocation;
	GLuint m_eyeWorldPosLocation;
	GLuint m_matSpecularIntensityLocation;
	GLuint m_matSpecularPowerLocation;
	GLuint m_numPointLightsLocation;
	GLuint m_numSpotLightsLocation;

	struct
	{
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Direction;
	} m_dirLightLocation;

	struct
	{
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Position;
		struct
		{
			GLuint Constant;
			GLuint Linear;
			GLuint Exp;
		} Atten;
	} m_pointLightsLocation[MAX_POINT_LIGHTS];

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Position;
		GLuint Direction;
		GLuint Cutoff;
		struct {
			GLuint Constant;
			GLuint Linear;
			GLuint Exp;
		} Atten;
	} m_spotLightsLocation[MAX_SPOT_LIGHTS];
};

class Camera;
class Mesh;
class Renderer;

class RenderTechnique
{
public:
	RenderTechnique();
	virtual bool Init() = 0;
	void Use();
	void Close();

	GLuint m_ShaderProgram;
protected:
	bool CreateProgram(const std::vector<Shader>& shaders, const std::string& fragout_identifier, GLuint frag_loc);
	
	bool GetLocation(GLuint& locationOut, const char* data);
	void setFloat_(GLuint* flt, float val);
	void setInt_(GLuint* i, GLint val);
	void setVec3_(GLuint* i, const Vec3& val);
	void setVec4_(GLuint* i, const Vec4& val);
	void setMat4_(GLuint* mat, int count, bool transpose, const Mat4& val);

private:
};

class LightTechnique : public RenderTechnique
{
public:
	static const unsigned int MAX_POINT_LIGHTS = 2;
	static const unsigned int MAX_SPOT_LIGHTS = 2;

	LightTechnique();
	bool Init() override;

	void setWVP(const Mat4& WVP);
	void setWorldMatrix(const Mat4& WVP);
	void setLightWVP(const Mat4& lwvp);

	void setTextureUnit(unsigned int textureUnit);
	void setShadowSampler(unsigned int sampler);

	void setDirectionalLight(const DirectionalLight& light);
	void setPointLights(unsigned int numLights, const PointLight* pLights);
	void setSpotLights(unsigned int numLights, const SpotLight* pLights);
	void setEyeWorldPos(const Vec3& eyeWorldPos);
	void setMatSpecularIntensity(float intensity);
	void setMatSpecularPower(float power);

private:
	GLuint m_Ufm_WvpXform;
	GLuint m_Ufm_WorldXform;
	GLuint m_Ufm_LightWvpXform;

	GLuint m_Ufm_Sampler;
	GLuint m_Ufm_ShadowSampler;

	GLuint m_Ufm_EyeWorldPos;
	GLuint m_Ufm_MatSpecularIntensity;
	GLuint m_Ufm_MatSpecularPower;
	GLuint m_Ufm_NumPointLights;
	GLuint m_Ufm_NumSpotLights;

	struct
	{
		GLuint colour;
		GLuint ambientIntensity;
		GLuint diffuseIntensity;
		GLuint direction;
	} m_Ufm_DirLight;

	struct
	{
		GLuint colour;
		GLuint ambientIntensity;
		GLuint diffuseIntensity;
		GLuint position;
		struct
		{
			GLuint constant;
			GLuint linear;
			GLuint exp;
		} Atten;
	} m_Ufm_PointLights[MAX_POINT_LIGHTS];

	struct {
		GLuint colour;
		GLuint ambientIntensity;
		GLuint diffuseIntensity;
		GLuint position;
		GLuint direction;
		GLuint cutoff;
		struct {
			GLuint constant;
			GLuint linear;
			GLuint exp;
		} Atten;
	} m_Ufm_SpotLights[MAX_SPOT_LIGHTS];

};

class FontTechnique : public RenderTechnique
{
public:
	FontTechnique();
	bool Init() override;
	
	void setColour(const Vec4& col);
	void setProjection(const Mat4& proj);

private:
	GLuint m_Ufm_ProjId;
	GLuint m_Ufm_ColId;
};

class SkyboxTechnique : public RenderTechnique
{
public:
	SkyboxTechnique();
	bool Init() override;

	void setWVP(const Mat4& WVP);
	void setTextureUnit(unsigned int TextureUnit);

	void Render(Camera* cam, Mesh* mesh, Renderer* r);

private:
	GLuint m_Ufm_WVP;
	GLuint m_Ufm_TexLoc;
};

class BasicDiffuseTechnique : public RenderTechnique
{
public:
	BasicDiffuseTechnique();
	bool Init() override;

	void setModelXform(const Mat4& m);
	void setProjXform(const Mat4& m);
	void setViewXform(const Mat4& m);

private:
	GLuint m_Ufm_ModelXform;
	GLuint m_Ufm_ViewXform;
	GLuint m_Ufm_ProjXform;
};

class ShadowMapTechnique : public RenderTechnique
{
public:
	ShadowMapTechnique();

	bool Init() override;
	void setWvpXform(const Mat4& wvp);
	void setTextureUnit(unsigned int textureUnit);

private:
	GLuint m_Ufm_WvpXform;
	GLuint m_Ufm_TextureLocation;
};


#endif