#ifndef __PLANE_H__
#define	__PLANE_H__

#include "types.h"

class Plane
{
public:
	Plane();

	void			Set(const Vec4& abcd);
	float			Distance(const Vec3& pos);
	const Vec3&		Normal() const;

private:
	friend class Frustum;
	Vec3 norm;
	float a, b, c, d;
};

#endif