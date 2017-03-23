#include "Transform.h"

#include "gl_headers.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
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

	this->model_xform =
		glm::translate(IDENTITY, this->position) *
		glm::yawPitchRoll(euler.y, euler.x, euler.z) *
		glm::scale(IDENTITY, this->scale);
}

void Transform::SetPosition(const Vec3& p)
{
	position = p;
}

void Transform::SetScale(const Vec3& p)
{
	scale = p;
}