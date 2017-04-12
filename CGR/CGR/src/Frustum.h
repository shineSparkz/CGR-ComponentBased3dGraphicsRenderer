#ifndef __FRUSTRUM_H__
#define __FRUSTUM_H__

#include <vector>
#include "AABB.h"
#include "Plane.h"
#include "gl_headers.h"

class Frustum
{
	enum {Near, Far, Left, Right, Top, Bottom };
public:
	Frustum();
	~Frustum();

	void UpdateFrustum			(const Mat4& proj, const Mat4& view);
	bool IsPointWithinFrustum	(const Vec3& pos);
	bool BoxInFrustum			(AABox &b, bool checkIntersections);
	bool SphereInFrustum		(const Vec3& centre, float r);

private:
	std::vector <Plane> planes{ 6 };
};
#endif