#include "ChaseCamera.h"
#include "EventManager.h"
#include "Time.h"
#include "Screen.h"
#include "Input.h"
#include "Transform.h"
#include "KeyEvent.h"

ChaseCamera2D::ChaseCamera2D(GameObject* go) :
	BaseCamera(go),
	m_Velocity(0.0f),
	m_MoveSpeed(565.0f),
	m_Fkey(false),
	m_Bkey(false),
	m_Lkey(false),
	m_Rkey(false)
{
	EventManager* ev = EventManager::Instance();

	if (ev)
	{
		ev->AttachEvent(EVENT_KEY, *this);
	}
}

ChaseCamera2D::~ChaseCamera2D()
{
	EventManager* ev = EventManager::Instance();

	if (ev)
	{
		ev->RemoveEvent(EVENT_KEY, *this);
	}
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
	m_Velocity = Vec3(0.0f);

	if (m_Lkey ^ m_Rkey)
	{
		m_Velocity = m_Lkey ? m_Velocity - (m_Right * m_MoveSpeed) : m_Velocity + (m_Right * m_MoveSpeed);
	}

	if (m_Fkey ^ m_Bkey)
	{
		m_Velocity = m_Fkey ? m_Velocity + Vec3(0, 1, 0) * m_MoveSpeed : m_Velocity - Vec3(0, 1, 0) * m_MoveSpeed;
	}

	// Update position
	m_Transform->MovePosition(m_Velocity * dt);

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
	}
	}
}