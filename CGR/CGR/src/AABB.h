#ifndef __AABB_H__
#define __AABB_H__

#include "types.h"

class AABox
{
public:
	AABox::AABox();
	AABox(const Vec3& min, const Vec3& max);
	AABox::~AABox();

	Vec3 min;
	Vec3 max;
};

#endif