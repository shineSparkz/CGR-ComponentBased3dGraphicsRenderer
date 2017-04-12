#include "AABB.h"

AABox::AABox()
	//corner(0.0f),
	//x(1.0f),
	//y(1.0f),
	//z(1.0f)
{
}

AABox::~AABox() 
{
}

AABox::AABox(const Vec3& min, const Vec3& max)
{
	this->min = min;
	this->max = max;
}

/*
AABox::AABox(const Vec3& corner, float x, float y, float z)
{
	setBox(corner, x, y, z);
}

void AABox::setBox(const Vec3& corner, float x, float y, float z)
{
	this->corner = corner;

	if (x < 0.0)
	{
		x = -x;
		this->corner.x -= x;
	}
	if (y < 0.0)
	{
		y = -y;
		this->corner.y -= y;
	}
	if (z < 0.0)
	{
		z = -z;
		this->corner.z -= z;
	}
	this->x = x;
	this->y = y;
	this->z = z;
}

Vec3 AABox::getVertexP(const Vec3& normal)
{
	Vec3 res = corner;

	if (normal.x > 0)
		res.x += x;

	if (normal.y > 0)
		res.y += y;

	if (normal.z > 0)
		res.z += z;

	return(res);
}

Vec3 AABox::getVertexN(const Vec3& normal)
{
	Vec3 res = corner;

	if (normal.x < 0)
		res.x += x;

	if (normal.y < 0)
		res.y += y;

	if (normal.z < 0)
		res.z += z;

	return(res);
}
*/