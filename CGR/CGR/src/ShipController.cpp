#include "ShipController.h"

#include "GameObject.h"
#include "Transform.h"
#include "Time.h"
#include "Camera.h"
#include "Input.h"
#include "utils.h"
#include "math_utils.h"
#include "LogFile.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/common.hpp>
#include <glm\gtx\normal.hpp>
#include "math_utils.h"


glm::quat RotationBetweenVectors(Vec3 start, Vec3 dest)
{
	start = normalize(start);
	dest = normalize(dest);

	float cosTheta = dot(start, dest);
	Vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f)
	{
		rotationAxis = glm::cross(Vec3(0.0f, 0.0f, 1.0f), start);
		
		if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
			rotationAxis = glm::cross(Vec3(1.0f, 0.0f, 0.0f), start);

		rotationAxis = normalize(rotationAxis);
		return glm::angleAxis(180.0f, rotationAxis);
	}

	rotationAxis = cross(start, dest);

	float s = sqrt((1 + cosTheta) * 2);
	float invs = 1 / s;

	return glm::quat(
		s * 0.5f,
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);

}

int ShipController::m_Id = SHIP_CONTROLLER_COMPONENT;

ShipController::ShipController(GameObject* owner) :
	Component(owner)
{
}

ShipController::~ShipController()
{
}

void ShipController::Start()
{
	m_Transform = gameObject()->GetComponent<Transform>();
	if (!m_Transform)
	{
		m_Transform = this->gameObject()->AddComponent<Transform>();
	}

	m_Transform->RotateY(-90.0f);
	m_Transform->UseQuatsForRotation(true);
}

void ShipController::SetCam(Transform* t)
{
	m_CamTransform = t;
}

void ShipController::Update()
{
	const float dt = Time::DeltaTime();
	const float SPEED = 90.0f;
	const float ROT_SPEED = 70.0f;

	const Vec3& e = m_Transform->Euler();

	if (Input::Keys[GLFW_KEY_W] == GLFW_PRESS || Input::Keys[GLFW_KEY_W] == GLFW_REPEAT)
	{
		m_Transform->MovePosition(Vec3(glm::sin(glm::radians(e.y - 90)) * 0.5f * (SPEED * dt), 0, glm::cos(glm::radians(e.y - 90)) * 0.5f * (SPEED * dt)));
	}
	if (Input::Keys[GLFW_KEY_S] == GLFW_PRESS || Input::Keys[GLFW_KEY_S] == GLFW_REPEAT)
	{
		m_Transform->MovePosition(Vec3(-glm::sin(glm::radians(-e.y + 90)) * 0.5f * (SPEED * dt), 0, -glm::cos(glm::radians(-e.y + 90)) * 0.5f * (SPEED * dt)));
	}
	if (Input::Keys[GLFW_KEY_A] == GLFW_PRESS || Input::Keys[GLFW_KEY_A] == GLFW_REPEAT)
	{
		m_Transform->RotateY(ROT_SPEED * dt);
	}
	if (Input::Keys[GLFW_KEY_D] == GLFW_PRESS || Input::Keys[GLFW_KEY_D] == GLFW_REPEAT)
	{
		m_Transform->RotateY(-ROT_SPEED * dt);
	}

	if (Input::Keys[GLFW_KEY_Q == GLFW_PRESS] || Input::Keys[GLFW_KEY_Q] == GLFW_REPEAT)
	{
		m_Transform->RotateX(ROT_SPEED * dt);
	}
	if (Input::Keys[GLFW_KEY_E] == GLFW_PRESS || Input::Keys[GLFW_KEY_E] == GLFW_REPEAT)
	{
		m_Transform->RotateX(-ROT_SPEED * dt);
	}

	if (Input::Keys[GLFW_KEY_SPACE == GLFW_PRESS] || Input::Keys[GLFW_KEY_SPACE] == GLFW_REPEAT)
	{
		m_Transform->MovePosition(Vec3(0, -SPEED * dt, 0));
	}
	if (Input::Keys[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS || Input::Keys[GLFW_KEY_LEFT_SHIFT] == GLFW_REPEAT)
	{
		m_Transform->MovePosition(Vec3(0, SPEED * dt, 0));
	}

	m_CamTransform->SetPosition(m_Transform->Position() + (Vec3(0, 35, 80)));
	
}