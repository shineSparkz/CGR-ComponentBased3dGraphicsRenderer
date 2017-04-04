#include "SpotLight.h"

#include "UniformBlockManager.h"
#include "UniformBlock.h"
#include "Renderer.h"
#include "LogFile.h"

int SpotLightC::m_Id = SPOT_LIGHT_COMPONENT;

SpotLightC::SpotLightC(GameObject* owner) :
	Component(owner),
	m_LightBlock(nullptr),
	m_Position(),
	m_Direction(),
	m_Intensity(),
	m_CosAngle(0.0f),
	m_aConstant(0.0f),
	m_aLinear(0.0f),
	m_aQuadratic(0.0f),
	m_LightIndex(-1),
	m_LightValid(GE_FALSE)
{
}

SpotLightC::~SpotLightC()
{
}

void SpotLightC::Start()
{
}

void SpotLightC::Update()
{
}

bool SpotLightC::SetLight(const Vec3& position,
	const Vec3& direction,
	const Vec3& intensity,
	float angle,
	float aConstant,
	float aLinear,
	float aQuadratic,
	int switchedOn
	)
{
	Renderer* ren = Renderer::Instance();
	if (!ren)
	{
		WRITE_LOG("Can't set a spot light without a renderer", "error");
		return false;
	}

	UniformBlockManager* ubm = UniformBlockManager::Instance();
	if (!ubm)
	{
		WRITE_LOG("Can't create spot light without uniform block manager, is the renderer created?", "error");
		return false;
	}

	m_LightBlock = ubm->GetBlock("Lights");
	if (!m_LightBlock)
	{
		WRITE_LOG("Can't create spot light without 'Lights' uniform blocks, is the renderer created?", "error");
		return false;
	}

	if (!m_LightBlock->IsBound())
	{
		WRITE_LOG("Can't create spot light without 'Lights' uniform blocks being bound", "error");
		m_LightBlock = nullptr;
		return false;
	}

	m_LightIndex = ren->GetSpotLightIndex();

	if (m_LightIndex < 0)
	{
		WRITE_LOG("Can't create spot light, exceeded max num spot lights", "warning");
		m_LightBlock = nullptr;
		return false;
	}

	m_Position = position;
	m_Direction = direction;
	m_Intensity = intensity;
	m_SwitchedOn = switchedOn;
	m_CosAngle = cosf(angle*3.1415f / 180.0f);
	m_aConstant = aConstant;
	m_aLinear = aLinear;
	m_aQuadratic = aQuadratic;
	m_LightValid = GE_TRUE;

	m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].position", &m_Position);
	m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].direction", &m_Direction);
	m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].intensity", &m_Intensity);
	m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].coneAngle", &m_CosAngle);
	m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].aConstant", &m_aConstant);
	m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].aLinear", &m_aLinear);
	m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].aQuadratic", &m_aQuadratic);
	m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].switched_on", &m_SwitchedOn);

	int num_spots = m_LightIndex + 1;
	m_LightBlock->SetValue("numSpots", &num_spots);
	return true;
}

void SpotLightC::SetPosition(const Vec3& pos)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Position = pos;
		m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].position", &m_Position);
	}
}

void SpotLightC::SetColour(const Vec3& col)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Intensity = col;
		m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].intensity", &m_Intensity);
	}
}

void SpotLightC::SetDirection(const Vec3& dir)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Direction = dir;
		m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].direction", &m_Direction);
	}
}

void SpotLightC::SetAngle(float angle)
{
	if (m_LightValid && m_LightBlock)
	{
		m_CosAngle = cosf(angle*3.1415f / 180.0f);
		m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].coneAngle", &m_CosAngle);
	}
}

void SpotLightC::ToggleLight()
{
	if (m_LightValid && m_LightBlock)
	{
		if (m_SwitchedOn > 0)
			m_SwitchedOn = 0;
		else
			m_SwitchedOn = 1;

		m_LightBlock->SetValue("spotLights[" + std::to_string(m_LightIndex) + "].switched_on", &m_SwitchedOn);
	}
}
