#include "Camera.h"
#include "Input.h"

#include "math_utils.h"
#include "EventManager.h"
#include "KeyEvent.h"
#include "Screen.h"
#include "GameObject.h"
#include "Transform.h"

int BaseCamera::m_Id = CAMERA_COMPONENT;

const Mat4 IDENTITY(1.0f);

BaseCamera::BaseCamera(GameObject* go) :
	Component(go),
	m_Transform(nullptr),
	projection(IDENTITY),
	view(IDENTITY),
	perspectiveSettings(),
	skyboxSettings(nullptr),
	up(0.0f),
	forward(0.0f),
	right(0.0f)
{
}

BaseCamera::~BaseCamera()
{
	SAFE_DELETE(this->skyboxSettings);
}

void BaseCamera::Start()
{
	if (!m_Transform)
	{
		m_Transform = this->gameObject()->AddComponent<Transform>();
		m_Transform->Start();
	}
}

void BaseCamera::Init(CamType type, const Vec3& position, const Vec3& up, const Vec3& right, const Vec3& forward, float fov, float aspect, float near, float far)
{
	this->camType = type;
	this->up = up;
	this->right = right;
	this->forward = forward;
	this->perspectiveSettings = PerspectiveSettings(fov, aspect, near, far);
	
	if (!m_Transform)
		m_Transform = gameObject()->AddComponent<Transform>();

	m_Transform->SetPosition(position);
	
	this->Update();
}

void BaseCamera::AddSkybox(float scale, unsigned cubeMapIndex)
{
	if (this->camType == CamType::Orthographic || this->camType == CamType::Shadow)
		return;

	if (!skyboxSettings)
	{
		skyboxSettings = new SkyboxSettings();
	}
	
	skyboxSettings->scale = scale;
	skyboxSettings->textureIndex = cubeMapIndex;
}

bool BaseCamera::HasSkybox() const
{
	return skyboxSettings != nullptr;
}

void BaseCamera::SetAspect(float asp)
{
	this->perspectiveSettings.aspect = asp;
}

void BaseCamera::SetNear(float near)
{
	this->perspectiveSettings.near = near;
}

void BaseCamera::SetFar(float far)
{
	this->perspectiveSettings.far = far;
}

void BaseCamera::SetFOV(float fov)
{
	this->perspectiveSettings.fov = fov;
}

void BaseCamera::SetPosition(const Vec3& p)
{
	m_Transform->SetPosition(p);
}

void BaseCamera::SetDirection(const Vec3& p)
{
	this->forward = p;
}

void BaseCamera::SetUp(const Vec3& u)
{
	this->up = u;
}

void BaseCamera::Update()
{
	if (this->camType == CamType::Perspective)
	{
		projection = glm::perspective(perspectiveSettings.fov, perspectiveSettings.aspect, perspectiveSettings.near, perspectiveSettings.far);
		view = glm::lookAt(m_Transform->Position(), m_Transform->Position() + forward, up);
	}
	else if (this->camType == CamType::Orthographic)
	{
		projection = glm::ortho(0.0f, (float)Screen::FrameBufferWidth(), 0.0f, (float)Screen::FrameBufferHeight(),
			perspectiveSettings.near, perspectiveSettings.far);
		view = glm::lookAt(m_Transform->Position(), m_Transform->Position() + forward, up);
	}
	else if (this->camType == CamType::Shadow)
	{
		const float fRangeX = 150, fRangeY = 150, fMinZ = 0.05f, fMaxZ = 400;

		projection = glm::ortho<float>(-fRangeX, fRangeX, -fRangeY, fRangeY, fMinZ, fMaxZ);
		
		Vec3 vLightPos = -forward * 150.0f;
		view = glm::lookAt(vLightPos, Vec3(0.0f), up);
		//view = glm::lookAt(m_Transform->Position(), m_Transform->Position() + forward, up);


		//float p = 100;
		//projection =  Maths::ortho_proj_transform(-p, p, -p, p, -p*0.1f , p);
			//0, (float)Screen::Instance()->FrameBufferWidth(), 0.0f, (float)Screen::Instance()->FrameBufferHeight(), this->perspectiveSettings.near, this->perspectiveSettings.far);
	}
	
}

const Vec3& BaseCamera::Position() const
{
	return this->m_Transform->Position();
}

const Vec3& BaseCamera::Up() const
{
	return this->up;
}

const Vec3& BaseCamera::Forward() const
{
	return this->forward;
}

const Vec3& BaseCamera::Right() const
{
	return this->right;
}

const Mat4& BaseCamera::Projection() const
{
	return this->projection;
}

const Mat4& BaseCamera::View() const
{
	return this->view;
}

const Mat4 BaseCamera::ProjXView() const
{
	return this->projection * this->view;
}

SkyboxSettings* BaseCamera::SkyBoxParams() const
{
	return skyboxSettings;
}


FlyCamera::FlyCamera(GameObject* go) :
	BaseCamera(go)
{
	EventManager* ev = EventManager::Instance();

	if (ev)
	{
		ev->AttachEvent(EVENT_KEY, *this);
		ev->AttachEvent(EVENT_WINDOW_FOCUS, *this);
	}
}

FlyCamera::~FlyCamera()
{
	EventManager* ev = EventManager::Instance();

	if (ev)
	{ 
		ev->RemoveEvent(EVENT_KEY, *this);
		ev->RemoveEvent(EVENT_WINDOW_FOCUS, *this);
	}
}

void FlyCamera::Start()
{
	BaseCamera::Start();
}

void FlyCamera::Update()
{
	if (this->windowFocused)
	{
		const float& dt = Time::DeltaTime();
		const float hscreenWidth = (float)Screen::ScreenWidth() * 0.5f;
		const float hscreenHeight = (float)Screen::ScreenHeight() * 0.5f;
		velocity = Vec3(0.0f);

		int num_dirs_pressed = ((rk ^ lk) ? 1 : 0) +
			((fk ^ bk) ? 1 : 0);

		if (lk ^ rk)
		{
			velocity = lk ? velocity - (right * speed) : velocity + (right * speed);
		}

		if (fk ^ bk)
		{
			velocity = fk ?
				velocity + forward * speed :
				velocity - forward * speed;
		}

		float mouseX = (float)Mouse::Instance()->PosX();
		float mouseY = (float)Mouse::Instance()->PosY();

		// Mouse orient
		m_Transform->RotateY(mouseSpeed * dt * (hscreenWidth - mouseX));
		m_Transform->RotateX(mouseSpeed * dt * (hscreenHeight - mouseY));

		Vec3 rot = m_Transform->Euler();
		this->forward = Vec3(cosf(rot.x) * sinf(rot.y), sinf(rot.x), cosf(rot.x) * cosf(rot.y));
		this->right = Vec3(sinf(rot.y - 3.14f / 2.0f), 0, cosf(rot.y - 3.14f / 2.0f));
		this->up = glm::cross(right, forward);

		// Update position
		m_Transform->MovePosition(this->velocity * dt);

		// Call base class
		BaseCamera::Update();
		Mouse::Instance()->SetMousePosition(hscreenWidth, hscreenHeight);
	}
}

void FlyCamera::HandleEvent(Event* e)
{
	switch (e->GetID())
	{
		case EVENT_KEY:
		{
			KeyEvent* ke = (KeyEvent*)e->GetData();

			if (ke)
			{
				if (ke->key == GLFW_KEY_W &&
					(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT) )
				{
					fk = true;
				}
				else
				{
					fk = false;
				}
				
				if (ke->key == GLFW_KEY_S &&
					(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
				{
					bk = true;
				}
				else
				{
					bk = false;
				}

				if (ke->key == GLFW_KEY_A &&
					(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
				{
					lk = true;
				}
				else
				{
					lk = false;
				}

				if (ke->key == GLFW_KEY_D &&
					(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
				{
					rk = true;
				}
				else
				{
					rk = false;
				}
			}

			break;
		}
		case EVENT_WINDOW_FOCUS:
		{
			int* param = (int*)e->GetData();
			if (param)
			{
				this->windowFocused = *param;
			}
			break;
		}
	}
}


ChaseCamera2D::ChaseCamera2D(GameObject* go) :
	BaseCamera(go)
{
	EventManager* ev = EventManager::Instance();

	if (ev)
	{
		ev->AttachEvent(EVENT_KEY, *this);
	}
}

ChaseCamera2D::~ChaseCamera2D()
{
}

void ChaseCamera2D::Start()
{
	BaseCamera::Start();
}

void ChaseCamera2D::Update()
{
	const float& dt = Time::DeltaTime();
	const float hscreenWidth = (float)Screen::ScreenWidth() * 0.5f;
	const float hscreenHeight = (float)Screen::ScreenHeight() * 0.5f;
	velocity = Vec3(0.0f);

	int num_dirs_pressed = ((rk ^ lk) ? 1 : 0) +
		((fk ^ bk) ? 1 : 0);

	if (lk ^ rk)
	{
		velocity = lk ? velocity - (right * speed) : velocity + (right * speed);
	}

	if (fk ^ bk)
	{
		velocity = fk ? velocity + Vec3(0,1,0) * speed : velocity - Vec3(0,1,0) * speed;
	}

	// Update position
	m_Transform->MovePosition(this->velocity * dt);

	// Call base class
	BaseCamera::Update();
}

void ChaseCamera2D::HandleEvent(Event* e)
{
	switch (e->GetID())
	{
	case EVENT_KEY:
	{
		KeyEvent* ke = (KeyEvent*)e->GetData();

		if (ke)
		{
			if (ke->key == GLFW_KEY_W &&
				(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
			{
				fk = true;
			}
			else
			{
				fk = false;
			}

			if (ke->key == GLFW_KEY_S &&
				(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
			{
				bk = true;
			}
			else
			{
				bk = false;
			}

			if (ke->key == GLFW_KEY_A &&
				(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
			{
				lk = true;
			}
			else
			{
				lk = false;
			}

			if (ke->key == GLFW_KEY_D &&
				(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
			{
				rk = true;
			}
			else
			{
				rk = false;
			}
		}
	}
	}
}