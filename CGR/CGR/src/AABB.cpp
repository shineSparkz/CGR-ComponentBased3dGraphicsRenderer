#include "AABB.h"

AABox::AABox() :
	min(0.0f),
	max(0.0f)
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
