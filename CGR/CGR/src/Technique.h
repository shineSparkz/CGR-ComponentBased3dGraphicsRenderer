#ifndef __TECHNIQUE_H__
#define __TECHNIQUE_H__

#include <list>
#include <vector>
#include <string>
#include "gl_headers.h"
#include "types.h"
#include "Lights.h"

#include "Shader.h"

class BaseCamera;
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
	void setVec2_(GLuint* i, const Vec2& val);
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
	void setNormalSampler(unsigned int sampler);

	void setEyeWorldPos(const Vec3& eyeWorldPos);
	void setMatSpecularIntensity(float intensity);
	void setMatSpecularPower(float power);

private:
	GLuint m_Ufm_WvpXform;
	GLuint m_Ufm_WorldXform;
	GLuint m_Ufm_LightWvpXform;

	GLuint m_Ufm_TextureSampler;
	GLuint m_Ufm_ShadowSampler;
	GLuint m_Ufm_NormalSampler;

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

class BillboardTechnique : public RenderTechnique
{
public:
	BillboardTechnique();

	bool Init() override;
	
	void setViewProjXform(const Mat4& vp);
	void setCamPos(const Vec3& pos);
	void setBillboardScale(float scale);
	void setTexureMapSampler(unsigned sampler);

private:
	GLuint m_Ufm_ViewProj;
	GLuint m_Ufm_CamPos;
	GLuint m_Ufm_Scale;
	GLuint m_Ufm_TextureMap;
};

class TerrainTechnique : public RenderTechnique
{
public:
	TerrainTechnique();

	bool Init() override;

	void setMatrices(BaseCamera* camera, const Mat4& model);

	void setShadowSampler(int samplerIndex);
	
	void setTexSampler(int arrayIndex, int textureIndex);
	void setColour(const Vec4& colour);
	
	void setHeightMapScaleXform(const Mat4& vp);
	void setRenderHeight(float val);
	void setMaxTexU(float val);
	void setMaxTexV(float val);

private:
	// Vert
	GLuint m_Ufm_HeightMapScaleXform;

	// Frag
	GLuint m_Ufm_Samplers[5];
	GLuint m_Ufm_ShadowMap;
	GLuint m_Ufm_Colour;
	GLuint m_Ufm_RenderHeight;
	GLuint m_Ufm_MaxTexU;
	GLuint m_Ufm_MaxTexV;

	struct
	{
		GLuint colour;
		GLuint ambientIntensity;
		GLuint direction;
	} m_Ufm_DirLight;

	struct
	{
		GLuint proj;
		GLuint model;
		GLuint view;
	} m_Ufm_Xforms;
};

class LavaTechnique : public RenderTechnique
{
public:
	LavaTechnique();
	~LavaTechnique();

	bool Init() override;

	void setWvpXform(const Mat4& wvp);
	void setTime(float time);
	void setTexSampler(int sampler);
	void setResolution(const Vec2& res);

private:
	GLuint m_Ufm_WVP;
	GLuint m_Ufm_Time;
	GLuint m_Ufm_TexSampler;
	GLuint m_Ufm_Resolution;
};

// ---- Deferred ----
class GeometryPassTechnique : public RenderTechnique
{
public:
	GeometryPassTechnique();
	~GeometryPassTechnique();

	bool Init() override;

	void setWVP(const Mat4& wvp);
	void setWorld(const Mat4& w);
	void setColourSampler(int sampler);

private:
	GLuint m_Ufm_WVP;
	GLuint m_Ufm_World;
	GLuint m_Ufm_ColourSampler;
};

class DSLightPassTech : public RenderTechnique
{
public:
	DSLightPassTech();

	bool Init();

	void setWVP(const Mat4& WVP);
	void setPositionTextureUnit(unsigned int textureUnit);
	void setColorTextureUnit(unsigned int textureUnit);
	void setNormalTextureUnit(unsigned int textureUnit);
	void setEyeWorldPos(const Vec3& eyeWorldPos);
	void setMatSpecularIntensity(float intensity);
	void setMatSpecularPower(float power);
	void setScreenSize(const Vec2& size);

private:
	GLuint m_WVPLocation;
	GLuint m_PosTextureUnitLocation;
	GLuint m_NormalTextureUnitLocation;
	GLuint m_ColorTextureUnitLocation;
	GLuint m_EyeWorldPosLocation;
	GLuint m_MatSpecularIntensityLocation;
	GLuint m_MatSpecularPowerLocation;
	GLuint m_ScreenSizeLocation;
};

class DSDirLightPassTech : public DSLightPassTech
{
public:
	DSDirLightPassTech();

	bool Init();

private:
	struct 
	{
		GLuint colour;
		GLuint ambientIntensity;
		GLuint diffuseIntensity;
		GLuint direction;
	} m_DirLightLocation;
};

class DSPointLightPassTech : public DSLightPassTech
{
public:
	DSPointLightPassTech();

	bool Init();

private:
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
	} m_PointLightLocation;
};

class NullTechnique : public RenderTechnique
{
public:
	NullTechnique();

	bool Init() override;
	void setWVP(const Mat4& WVP);

private:
	GLuint m_WVPLocation;
};

#endif