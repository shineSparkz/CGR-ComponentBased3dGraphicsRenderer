#ifndef __AABB_H__
#define __AABB_H__

#include "types.h"

class AABox
{
public:
	AABox::AABox();
	AABox(const Vec3& min, const Vec3& max);
	AABox::~AABox();
	
	//AABox::AABox(const Vec3& corner, float x, float y, float z);
	//void AABox::setBox(const Vec3& corner, float x, float y, float z);
	//Vec3 AABox::getVertexP(const Vec3& normal);
	//Vec3 AABox::getVertexN(const Vec3& normal);

	//Vec3 corner;
	//float x, y, z;
	Vec3 min;
	Vec3 max;
};

#endif