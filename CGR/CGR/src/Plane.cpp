#include "Plane.h"

Plane::Plane() :
	a(0.0f),
	b(0.0f),
	c(0.0f),
	d(0.0f)
{
}

float Plane::Distance(const Vec3& pos)
{
	return glm::dot(Vec3(a, b, c), pos) + d;
}

void Plane::Set(const Vec4& v)
{
	a = v.x;
	b = v.y;
	c = v.z;
	d = v.w;
	norm = glm::normalize(Vec3(a, b, c));
}

const Vec3& Plane::Normal() const
{
	return norm;
}