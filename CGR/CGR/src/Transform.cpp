#include "Transform.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include "math_utils.h"

int Transform::m_Id = TRANSFORM_COMPONENT;

const Mat4 IDENTITY(1.0f);

Transform::Transform(GameObject* go) :
	Component(go),
	position(0.0f),
	scale(1.0f),
	euler(0.0f),
	model_xform(Mat4(1.0f))
{
}

Transform::~Transform()
{
}

void Transform::Start()
{
}

void Transform::Update()
{
	//model_xform = IDENTITY;
	//model_xform = glm::translate(model_xform, this->position);
	//model_xform = (glm::rotate(model_xform, euler.z, Vec3(0, 0, 1)));
	//model_xform = (glm::rotate(model_xform, euler.y, Vec3(0,1,0)));
	//model_xform = (glm::rotate(model_xform, euler.x, Vec3(1, 0, 0))); 
	//model_xform = glm::mat4_cast(glm::rotation(position, euler));
	//model_xform = glm::scale(model_xform, this->scale);

	if (use_quats)
	{
		Mat4 t = glm::translate(IDENTITY, this->position);
		Mat4 r =
			glm::mat4_cast(glm::angleAxis(glm::radians(euler.z), Vec3(0, 0, 1))) *
			glm::mat4_cast(glm::angleAxis(glm::radians(euler.y), Vec3(0, 1, 0))) *
			glm::mat4_cast(glm::angleAxis(glm::radians(euler.x), Vec3(1, 0, 0)));
		Mat4 s = glm::scale(IDENTITY, scale);

		model_xform = t * r * s;
	}
	else
	{
		this->model_xform =
			glm::translate(IDENTITY, this->position) *
			glm::yawPitchRoll(euler.y, euler.x, euler.z) *
			glm::scale(IDENTITY, this->scale);
	}
}

void Transform::SetPosition(const Vec3& p)
{
	position = p;
}

void Transform::SetScale(const Vec3& p)
{
	scale = p;
}