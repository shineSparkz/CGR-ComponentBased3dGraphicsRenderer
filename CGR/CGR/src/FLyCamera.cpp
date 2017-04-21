#include "FlyCamera.h"
#include "EventManager.h"
#include "Time.h"
#include "Screen.h"
#include "Input.h"
#include "Transform.h"
#include "KeyEvent.h"

FlyCamera::FlyCamera(GameObject* go) :
	BaseCamera(go),
	m_Velocity(0.0f),
	m_MoveSpeed(80.0f),
	m_MouseSpeed(1.4f),
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

void FlyCamera::SetSpeeds(float moveSpeed, float mouseSpeed)
{
	m_MoveSpeed = moveSpeed;
	m_MouseSpeed = mouseSpeed;
}

void FlyCamera::Update()
{
	if (m_WindowFocused)
	{
		const float& dt = Time::DeltaTime();

		const float hscreenWidth =  (float)Screen::ScreenWidth()  * 0.5f;
		const float hscreenHeight = (float)Screen::ScreenHeight() * 0.5f;
		const float mouseX = (float)Mouse::Instance()->PosX();
		const float mouseY = (float)Mouse::Instance()->PosY();

		m_Velocity = Vec3(0.0f);

		if (m_Lkey ^ m_Rkey)
		{
			m_Velocity = m_Lkey ? m_Velocity - (m_Right * m_MoveSpeed) : m_Velocity + (m_Right * m_MoveSpeed);
		}

		if (m_Fkey ^ m_Bkey)
		{
			m_Velocity = m_Fkey ?
				m_Velocity + (m_Forward * m_MoveSpeed) :
				m_Velocity - (m_Forward * m_MoveSpeed);
		}

		// Mouse orient
		m_Transform->RotateY(m_MouseSpeed * dt * (hscreenWidth - mouseX));
		m_Transform->RotateX(m_MouseSpeed * dt * (hscreenHeight - mouseY));

		Vec3 rot = m_Transform->Euler();
		m_Forward = Vec3(cosf(rot.x) * sinf(rot.y), sinf(rot.x), cosf(rot.x) * cosf(rot.y));
		m_Right = Vec3(sinf(rot.y - 3.14f / 2.0f), 0, cosf(rot.y - 3.14f / 2.0f));
		m_Up = glm::cross(m_Right, m_Forward);

		// Update position
		m_Transform->MovePosition(m_Velocity * dt);

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