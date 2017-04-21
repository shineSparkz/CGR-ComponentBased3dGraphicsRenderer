#include "Camera.h"
#include "Input.h"

#include "math_utils.h"
#include "KeyEvent.h"
#include "Screen.h"
#include "GameObject.h"
#include "Transform.h"

int BaseCamera::m_Id = CAMERA_COMPONENT;

const Mat4 IDENTITY(1.0f);


BaseCamera::BaseCamera(GameObject* go) :
	Component(go),
	m_Transform(nullptr),
	m_Up(0.0f),
	m_Forward(0.0f),
	m_Right(0.0f),
	m_CamType(CamType::Perspective),
	m_Projection(IDENTITY),
	m_View(IDENTITY),
	m_SkyboxSettings(nullptr),
	m_PerspectiveSettings()
{
}

BaseCamera::~BaseCamera()
{
	SAFE_DELETE(m_SkyboxSettings);
}

void BaseCamera::Start()
{
	if (!m_Transform)
	{
		m_Transform = this->gameObject()->AddComponent<Transform>();
		m_Transform->Start();
	}
}

void BaseCamera::Update()
{
	if (m_CamType == CamType::Perspective)
	{
		m_Projection = glm::perspective(
			m_PerspectiveSettings.fov, 
			m_PerspectiveSettings.aspect, 
			m_PerspectiveSettings.near, 
			m_PerspectiveSettings.far
		);

		m_View = glm::lookAt(m_Transform->Position(), m_Transform->Position() + m_Forward, m_Up);
	}
	else if (m_CamType == CamType::Orthographic)
	{
		m_Projection = glm::ortho(0.0f, (float)Screen::FrameBufferWidth(), 0.0f, (float)Screen::FrameBufferHeight(),
			m_PerspectiveSettings.near, m_PerspectiveSettings.far);
		m_View = glm::lookAt(m_Transform->Position(), m_Transform->Position() + m_Forward, m_Up);
	}
	else if (m_CamType == CamType::Light)
	{
		const float fRangeX = 150, fRangeY = 150, fMinZ = 0.05f, fMaxZ = 400;
		m_Projection = glm::ortho<float>(-fRangeX, fRangeX, -fRangeY, fRangeY, fMinZ, fMaxZ);
		Vec3 vLightPos = -m_Forward * 150.0f;
		m_View = glm::lookAt(vLightPos, Vec3(0.0f), m_Up);
	}
}

void BaseCamera::Init(CamType type, const Vec3& position, const Vec3& up, const Vec3& right, const Vec3& forward, float fov, float aspect, float near, float far)
{
	m_CamType = type;
	m_Up = up;
	m_Right = right;
	m_Forward = forward;
	m_PerspectiveSettings = PerspectiveSettings(fov, aspect, near, far);
	
	if (!m_Transform)
		m_Transform = gameObject()->AddComponent<Transform>();

	m_Transform->SetPosition(position);
	
	this->Update();
}

void BaseCamera::AddSkybox(float scale, unsigned cubeMapIndex)
{
	if (m_CamType == CamType::Orthographic || m_CamType == CamType::Light)
		return;

	if (!m_SkyboxSettings)
	{
		m_SkyboxSettings = new SkyboxSettings();
	}
	
	m_SkyboxSettings->scale = scale;
	m_SkyboxSettings->textureIndex = cubeMapIndex;
}


void BaseCamera::SetAspect(float asp)
{
	m_PerspectiveSettings.aspect = asp;
}

void BaseCamera::SetNear(float near)
{
	m_PerspectiveSettings.near = near;
}

void BaseCamera::SetFar(float far)
{
	m_PerspectiveSettings.far = far;
}

void BaseCamera::SetFOV(float fov)
{
	m_PerspectiveSettings.fov = fov;
}

void BaseCamera::SetPosition(const Vec3& p)
{
	m_Transform->SetPosition(p);
}

void BaseCamera::SetDirection(const Vec3& p)
{
	m_Forward = p;
}

void BaseCamera::SetUp(const Vec3& u)
{
	m_Up = u;
}

bool BaseCamera::HasSkybox() const
{
	return m_SkyboxSettings != nullptr;
}

SkyboxSettings* BaseCamera::SkyBoxParams() const
{
	return m_SkyboxSettings;
}

const Vec3& BaseCamera::Position() const
{
	return this->m_Transform->Position();
}

const Vec3& BaseCamera::Up() const
{
	return m_Up;
}

const Vec3& BaseCamera::Forward() const
{
	return m_Forward;
}

const Vec3& BaseCamera::Right() const
{
	return m_Right;
}

const Mat4& BaseCamera::Projection() const
{
	return m_Projection;
}

const Mat4& BaseCamera::View() const
{
	return m_View;
}

const Mat4 BaseCamera::ProjXView() const
{
	return m_Projection * m_View;
}

const CamType BaseCamera::GetProjectionType() const
{
	return m_CamType;
}