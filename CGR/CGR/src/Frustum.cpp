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
	Mat4 temp;
	temp = ( proj * view ) ;

	/*
	static int checked = 1;

	if ( checked  > 0)
	{
	std::cout << "Print all\n\n";
	for ( int i = 0; i < 4; ++i )
	{
	for ( int j = 0; j < 4; ++j )
	{
	std::cout << "[" << temp[i][j] << "] , ";
	}
	std::cout << "\n";
	}

	std::cout << "\n\nPrint Row:\n" ;
	glm::vec4 a = temp[3];
	for ( int j = 0; j < 4; ++j )
	{
	std::cout << "[" << a[j] << "] , ";
	}

	std::cout << "\n\nPrint Indiv:\n";
	std::cout << temp[0][3] << ", " << temp[1][3] << ", " << temp[2][3] << ", " << temp[3][3] << "\n----------------\n\naa";

	--checked;
	}
	*/

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

	
	/*
	Vec4 p = glm::normalize(Vec4(planes[Near].a, planes[Near].b, planes[Near].c, planes[Far].d));
	Vec4 p0 = glm::inverse(proj * view) * Vec4(p.x, p.y, p.z, 1.0);
	p0 = p0 * 1.0f / p0.w;
	
	float hw = 1280 / 2;
	float hh = 720 / 2;
	float far = 0.2f;
	float near = 0.f;
	Vec3 up = glm::normalize(camUp);
	Vec3 fwd = glm::normalize(camFwd);
	Vec3 right = glm::normalize(camRight);
	this->points[0] = Vec3(p0) + Vec3(-hw * right.x,  hh * up.y, far * fwd.z);	// TL
	this->points[1] = Vec3(p0) + Vec3( hw * right.x,  hh * up.y, far * fwd.z);	// TR
	this->points[2] = Vec3(p0) + Vec3(-hw * right.x, -hh * up.y, far * fwd.z);	// BL
	this->points[3] = Vec3(p0) + Vec3( hw * right.x, -hh * up.y, far * fwd.z);	// BR
	*/
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
		//if ( ((*i).Distance(b.min)) < 0.0f) && ((*i).Distance(b.max) < 0.0f)
		//	 ((*i).Distance(b.min) < 0.0f) && ((*i).Distance(b.max) < 0.0f) )
		{
			return false;
		}

		/*
		if ((*i).Distance(b.getVertexP((*i).Normal())) < 0)
			return false;

		// Intersections
		else if ((*i).Distance(b.getVertexN((*i).Normal())) < 0)
		{
			return checkIntersections;
		}
		*/
	}

	return true;
}