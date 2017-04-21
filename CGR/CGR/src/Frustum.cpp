#include "Frustum.h"

#include "math_utils.h"

Frustum::Frustum()
{
}

Frustum::~Frustum()
{
}

void Frustum::UpdateFrustum(const Mat4& proj, const Mat4& view)
{
	Mat4 temp = ( proj * view ) ;

	// Near plane (Z' - =1 ) [ Col 3 + Col 4 ]
	planes[Near].Set(
		Vec4(
			temp[0][3] + temp[0][2],
			temp[1][3] + temp[1][2],
			temp[2][3] + temp[2][2],
			temp[3][3] + temp[3][2]));

	// Far plane  (Z' = 1 ) [ Col 3 + Col 4 ]
	planes[Far].Set(
		Vec4(
			temp[0][3] - temp[0][2],
			temp[1][3] - temp[1][2],
			temp[2][3] - temp[2][2],
			temp[3][3] - temp[3][2]));
	
	// Left plane ( X' = -1 )
	planes[Left].Set(
		Vec4(
			temp[0][3] + temp[0][0],
			temp[1][3] + temp[1][0],
			temp[2][3] + temp[2][0],
			temp[3][3] + temp[3][0]));


	// Right plane  ( X' = 1 )
	planes[Right].Set(
		Vec4(
			temp[0][3] - temp[0][0],
			temp[1][3] - temp[1][0],
			temp[2][3] - temp[2][0],
			temp[3][3] - temp[3][0]));

	// Top plane(Y' = 1 )
	planes[Top].Set(
		Vec4(
			temp[0][3] - temp[0][1],
			temp[1][3] - temp[1][1],
			temp[2][3] - temp[2][1],
			temp[3][3] - temp[3][1]));


	// Bottom plane (Y' = -1 )
	planes[Bottom].Set(
		Vec4(
			temp[0][3] + temp[0][1],
			temp[1][3] + temp[1][1],
			temp[2][3] + temp[2][1],
			temp[3][3] + temp[3][1]));
}

bool Frustum::IsPointWithinFrustum(const Vec3& pos)
{
	for (auto i = planes.begin(); i != planes.end(); ++i)
	{
		if ((*i).Distance(pos) < 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool Frustum::SphereInFrustum(const Vec3& centre, float r)
{
	for (auto i = planes.begin(); i != planes.end(); ++i)
	{
		if ((*i).Distance(centre) <= -r)
		{
			return false;
		}
	}

	return true;
}

bool Frustum::BoxInFrustum(AABox &b, bool checkIntersections)
{
	for (auto i = planes.begin(); i != planes.end(); ++i)
	{
		if( ( (*i).Distance(b.min) < 0.0f ) )
		{
			return false;
		}
	}

	return true;
}