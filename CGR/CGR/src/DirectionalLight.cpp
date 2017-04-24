#include "DirectionalLight.h"

#include "UniformBlockManager.h"
#include "UniformBlock.h"
#include "Renderer.h"
#include "LogFile.h"

int DirectionalLightC::m_Id = DIRECTION_LIGHT_COMPONENT;

DirectionalLightC::DirectionalLightC(GameObject* owner) :
	Component(owner),
	m_LightBlock(nullptr),
	m_Direction(),
	m_Intensity(),
	m_LightValid(GE_FALSE)
{
}

DirectionalLightC::~DirectionalLightC()
{
}

void DirectionalLightC::Start()
{
}

bool DirectionalLightC::SetLight(Renderer* renderer, const Vec3& direction, const Vec3& intensity, const Vec3& range)
{
	if (!renderer)
	{
		WRITE_LOG("Can't set a directional light without a renderer", "error");
		return false;
	}

	UniformBlockManager* ubm = UniformBlockManager::Instance();

	if (!ubm)
	{
		WRITE_LOG("Can't create directional light without uniform block manager, is the renderer created?", "error");
		return false;
	}

	m_LightBlock = ubm->GetBlock("Lights");

	if (!m_LightBlock)
	{
		WRITE_LOG("Can't create directional light without 'Lights' uniform blocks, is the renderer created?", "error");
		return false;
	}

	if (!m_LightBlock->IsBound())
	{
		WRITE_LOG("Can't create directional light without 'Lights' uniform blocks being bound", "error");
		m_LightBlock = nullptr;
		return false;
	}

	if (renderer->GetDirLightIndex() < 0)
	{
		WRITE_LOG("Dirctional light not set because there is aready one in scene", "warning");
		m_LightBlock = nullptr;
		return false;
	}

	m_Renderer = renderer;
	m_Direction = direction;
	m_Intensity = intensity;
	m_Range = range;
	m_LightValid = GE_TRUE;

	m_LightBlock->SetValue("directionLight.direction", (void*)glm::value_ptr(m_Direction));
	m_LightBlock->SetValue("directionLight.intensity", (void*)glm::value_ptr(m_Intensity));

	m_Renderer->UpdateDirLight(m_Direction, m_Range);
	return true;
}

void DirectionalLightC::Update()
{
}

void DirectionalLightC::SetDirection(const Vec3& newDir)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Direction = newDir;
		m_LightBlock->SetValue("directionLight.direction", (void*)glm::value_ptr(m_Direction));
		m_Renderer->UpdateDirLight(m_Direction, m_Range);
	}
}

void DirectionalLightC::SetRange(const Vec3& range)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Range = range;
		m_Renderer->UpdateDirLight(m_Direction, m_Range);
	}
}

void DirectionalLightC::SetDirectionX(float x)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Direction.x = x;
		m_LightBlock->SetValue("directionLight.direction", (void*)glm::value_ptr(m_Direction));
		m_Renderer->UpdateDirLight(m_Direction, m_Range);
	}
}

void DirectionalLightC::SetDirectionY(float y)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Direction.y = y;
		m_LightBlock->SetValue("directionLight.direction", (void*)glm::value_ptr(m_Direction));
		m_Renderer->UpdateDirLight(m_Direction, m_Range);
	}
}

void DirectionalLightC::SetDirectionZ(float z)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Direction.z = z;
		m_LightBlock->SetValue("directionLight.direction", (void*)glm::value_ptr(m_Direction));
		m_Renderer->UpdateDirLight(m_Direction, m_Range);
	}
}

void DirectionalLightC::SetColour(const Vec3& newCol)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Intensity = newCol;
		m_LightBlock->SetValue("directionLight.intensity", (void*)glm::value_ptr(m_Intensity));
	}
}

void DirectionalLightC::SetColR(float r)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Intensity.r = r;
		m_LightBlock->SetValue("directionLight.intensity", (void*)glm::value_ptr(m_Intensity));
	}
}

void DirectionalLightC::SetColG(float g)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Intensity.g = g;
		m_LightBlock->SetValue("directionLight.intensity", (void*)glm::value_ptr(m_Intensity));
	}
}

void DirectionalLightC::SetColB(float b)
{
	if (m_LightValid && m_LightBlock)
	{
		m_Intensity.b = b;
		m_LightBlock->SetValue("directionLight.intensity", (void*)glm::value_ptr(m_Intensity));
	}
}

const Vec3& DirectionalLightC::GetDir() const
{
	return m_Direction;
}

const Vec3& DirectionalLightC::GetRange() const
{
	return m_Range;
}

