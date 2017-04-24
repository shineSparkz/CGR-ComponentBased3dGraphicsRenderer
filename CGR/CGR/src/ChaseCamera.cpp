#include "ChaseCamera.h"

#include "math_utils.h"
#include "EventManager.h"
#include "Time.h"
#include "Screen.h"
#include "Input.h"
#include "Transform.h"
#include "KeyEvent.h"
#include "Transform.h"

ChaseCamera2D::ChaseCamera2D(GameObject* go) :
	BaseCamera(go)
{
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
	
	if (m_Target)
	{
		const Vec3& tp = m_Target->Position();
		m_Transform->SetPosition( Maths::LerpV3(m_Transform->Position(),  Vec3(tp.x - hscreenWidth, tp.y - hscreenHeight, m_Transform->Position().z), Time::DeltaTime()));
	}

	// Call base class
	BaseCamera::Update();
}