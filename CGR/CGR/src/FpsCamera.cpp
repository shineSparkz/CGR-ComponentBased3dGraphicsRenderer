#include "FpsCamera.h"

#include "math_utils.h"
#include "EventManager.h"
#include "Time.h"
#include "Screen.h"
#include "Input.h"
#include "Transform.h"
#include "KeyEvent.h"
#include "Terrain.h"

FpsCamera::FpsCamera(GameObject* go) :
	BaseCamera(go),
	m_Velocity(0.0f),
	m_MoveSpeed(30.0f),
	m_MouseSpeed(1.2f),
	m_WindowFocused(1),
	m_Fkey(false),
	m_Bkey(false),
	m_Lkey(false),
	m_Rkey(false)
{
	EventManager* ev = EventManager::Instance();

	if (ev)
	{
		ev->AttachEvent(EVENT_KEY, *this);
		ev->AttachEvent(EVENT_WINDOW_FOCUS, *this);
	}
}

FpsCamera::~FpsCamera()
{
	EventManager* ev = EventManager::Instance();

	if (ev)
	{
		ev->RemoveEvent(EVENT_KEY, *this);
		ev->RemoveEvent(EVENT_WINDOW_FOCUS, *this);
	}
}

void FpsCamera::Start()
{
	BaseCamera::Start();
}

void FpsCamera::Update()
{
	if (m_WindowFocused)
	{
		const float& dt = Time::DeltaTime();

		const float hscreenWidth = (float)Screen::ScreenWidth()  * 0.5f;
		const float hscreenHeight = (float)Screen::ScreenHeight() * 0.5f;
		const float mouseX = (float)Mouse::Instance()->PosX();
		const float mouseY = (float)Mouse::Instance()->PosY();

		m_Velocity = Vec3(0.0f);

		if (m_Lkey ^ m_Rkey)
		{
			//m_Velocity = m_Lkey ? m_Velocity - (m_Right * m_MoveSpeed) : m_Velocity + (m_Right * m_MoveSpeed);
			m_Velocity = m_Lkey ? 
				m_Velocity + glm::cross(Vec3(0,1,0), m_Forward) * m_MoveSpeed :
				m_Velocity - glm::cross(Vec3(0,1,0), m_Forward) * m_MoveSpeed;
		}

		if (m_Fkey ^ m_Bkey)
		{
			// The hacky 0.5f y value is to solve collision sliding equations quickly
			m_Velocity = m_Fkey ?
				m_Velocity + (Vec3(m_Forward.x, 0.5f, m_Forward.z) * m_MoveSpeed) :
				m_Velocity - (Vec3(m_Forward.x, 0.5f, m_Forward.z) * m_MoveSpeed);
		}

		// Mouse orient
		m_Transform->RotateY(m_MouseSpeed * dt * (hscreenWidth - mouseX));
		m_Transform->RotateX(m_MouseSpeed * dt * (hscreenHeight - mouseY));

		Vec3 rot = m_Transform->Euler();
		m_Forward = Vec3(cosf(rot.x) * sinf(rot.y), sinf(rot.x), cosf(rot.x) * cosf(rot.y));
		m_Right = Vec3(sinf(rot.y - 3.14f / 2.0f), 0, cosf(rot.y - 3.14f / 2.0f));
		m_Up = glm::cross(m_Right, m_Forward);

		// Collision stuff
		CollisionPacket cameraCP;
		cameraCP.ellipsoidSpace = Vec3(1.0f, 7.0f, 1.0f);
		cameraCP.w_pos = m_Transform->Position();
		cameraCP.w_vel = m_Velocity * dt;

		m_Previous = m_Transform->Position();
		m_Transform->SetPosition(Maths::LerpV3(m_Previous,  m_Terrain->CollisionSlide(cameraCP), dt * m_MoveSpeed));

		// Crude grounding
		if (m_Transform->Position().y < 0.0f)
			m_Transform->SetPosition(Vec3(m_Transform->Position().x, 0, m_Transform->Position().z));

		// Call base class
		BaseCamera::Update();
		Mouse::Instance()->SetMousePosition(hscreenWidth, hscreenHeight);
	}
}

void FpsCamera::HandleEvent(Event* e)
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
				m_Fkey = true;
			}
			else
			{
				m_Fkey = false;
			}

			if (ke->key == GLFW_KEY_S &&
				(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
			{
				m_Bkey = true;
			}
			else
			{
				m_Bkey = false;
			}

			if (ke->key == GLFW_KEY_A &&
				(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
			{
				m_Lkey = true;
			}
			else
			{
				m_Lkey = false;
			}

			if (ke->key == GLFW_KEY_D &&
				(ke->action == GLFW_PRESS || ke->action == GLFW_REPEAT))
			{
				m_Rkey = true;
			}
			else
			{
				m_Rkey = false;
			}
		}

		break;
	}
	case EVENT_WINDOW_FOCUS:
	{
		int* param = (int*)e->GetData();
		if (param)
		{
			this->m_WindowFocused = *param;
		}
		break;
	}
	}
}