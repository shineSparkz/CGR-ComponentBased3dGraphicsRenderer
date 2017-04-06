#include "PointLight.h"

#include "UniformBlockManager.h"
#include "UniformBlock.h"
#include "Renderer.h"
#include "LogFile.h"

int PointLightC::m_Id = POINT_LIGHT_COMPONENT;

PointLightC::PointLightC(GameObject* owner) :
	Component(owner),
	m_LightBlock(nullptr),
	m_Position(),
	m_Intensity(),
	m_AmbientIntensity(0.0f),
	m_aConstant(0.0f),
	m_aLinear(0.0f),
	m_aQuadratic(0.0f),
	m_LightIndex(-1),
	m_LightValid(GE_FALSE)
{
}

PointLightC::~PointLightC()
{
}

void PointLightC::Start()
{
}

void PointLightC::Update()
{
}

bool PointLightC::SetLight(const Vec3& position,
	const Vec3& intensity,
	float ambIntensity,
	float aConstant,
	float aLinear,
	float aQuadratic)
{
	Renderer* ren = Renderer::Instance();
	if (!ren)
	{
		WRITE_LOG("Can't set a point light without a renderer", "error");
		return false;
	}

	UniformBlockManager* ubm = UniformBlockManager::Instance();
	if (!ubm)
	{
		WRITE_LOG("Can't create point light without uniform block manager, is the renderer created?", "error");
		return false;
	}

	m_LightBlock = ubm->GetBlock("Lights");
	if (!m_LightBlock)
	{
		WRITE_LOG("Can't create point light without 'Lights' uniform blocks, is the renderer created?", "error");
		return false;
	}

	if (!m_LightBlock->IsBound())
	{
		WRITE_LOG("Can't create point light without 'Lights' uniform blocks being bound", "error");
		m_LightBlock = nullptr;
		return false;
	}

	m_LightIndex = ren->GetPointLightIndex();

	if (m_LightIndex < 0)
	{
		WRITE_LOG("Can't create point light, exceeded max num point lights", "warning");
		m_LightBlock = nullptr;
		return false;
	}

	m_Position = position;
	m_Intensity = intensity;
	m_AmbientIntensity = ambIntensity;
	m_aConstant = aConstant;
	m_aLinear = aLinear;
	m_aQuadratic = aQuadratic;
	m_LightValid = GE_TRUE;
	m_Range = calcPointLightBSphere();

	m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].position", &m_Position);
	m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].intensity", &m_Intensity);
	m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].ambient_intensity", &m_AmbientIntensity);
	m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].aConstant", &m_aConstant);
	m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].aLinear", &m_aLinear);
	m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].aQuadratic", &m_aQuadratic);

	int num_points = m_LightIndex + 1;
	m_LightBlock->SetValue("numPoints", &num_points);
	Renderer::Instance()->UpdatePointLight(m_LightIndex, m_Position, m_Range);
}

void PointLightC::SetPosition(const Vec3& pos)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Position = pos;
		m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].position", &m_Position);
		Renderer::Instance()->UpdatePointLight(m_LightIndex, m_Position, m_Range);
	}
}

void PointLightC::SetPositionX(float x)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Position.x = x;
		m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].position", &m_Position);
		Renderer::Instance()->UpdatePointLight(m_LightIndex, m_Position, m_Range);
	}
}

void PointLightC::SetPositionY(float y)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Position.y = y;
		m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].position", &m_Position);
		Renderer::Instance()->UpdatePointLight(m_LightIndex, m_Position, m_Range);
	}
}

void PointLightC::SetPositionZ(float z)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Position.z = z;
		m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].position", &m_Position);
		Renderer::Instance()->UpdatePointLight(m_LightIndex, m_Position, m_Range);
	}
}

void PointLightC::SetColour(const Vec3& newCol)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Intensity = newCol;
		m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].intensity", &m_Intensity);
		m_Range = calcPointLightBSphere();
		Renderer::Instance()->UpdatePointLight(m_LightIndex, m_Position, m_Range);
	}
}

void PointLightC::SetColR(float r)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Intensity.r = r;
		m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].intensity", &m_Intensity);
		m_Range = calcPointLightBSphere();
		Renderer::Instance()->UpdatePointLight(m_LightIndex, m_Position, m_Range);
	}
}

void PointLightC::SetColG(float g)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Intensity.g = g;
		m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].intensity", &m_Intensity);
		m_Range = calcPointLightBSphere();
		Renderer::Instance()->UpdatePointLight(m_LightIndex, m_Position, m_Range);
	}
}

void PointLightC::SetColB(float b)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Intensity.b = b;
		m_LightBlock->SetValue("pointLights[" + std::to_string(m_LightIndex) + "].intensity", &m_Intensity);
		m_Range = calcPointLightBSphere();
		Renderer::Instance()->UpdatePointLight(m_LightIndex, m_Position, m_Range);
	}
}

float PointLightC::calcPointLightBSphere()
{
	//float lightMax = std::fmaxf(std::fmaxf(m_Intensity.r, m_Intensity.g), m_Intensity.b);
	//return(-m_aLinear + std::sqrtf(m_aLinear * m_aLinear - 4 * m_aQuadratic * (m_aConstant - (256.0f / 5.0f) * lightMax))) / (2 * m_aQuadratic);

	float maxChannel = fmax(fmax(m_Intensity.x, m_Intensity.y), m_Intensity.z);
	return (-m_aLinear + sqrtf(m_aLinear * m_aLinear -	4 * m_aQuadratic * (m_aQuadratic - 256 * maxChannel * m_AmbientIntensity))) / (2 * m_aQuadratic);
}
