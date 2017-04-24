#pragma once
#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__

#include "types.h"
#include "Vertex.h"

#include <cmath>

namespace Maths
{
	#define EPSILON  1.192092896e-07f

	INLINE float  ToDegrees(float radians)
	{
		return float(radians * (180.0f / 3.1415926535f) );
	}

	INLINE float  ToRadians(float degrees)
	{
		return float(degrees * (3.1415926535f / 180.0f ));
	}

	template<class T> T	Max(T x, T y)
	{
		return (x > y) ? x : y;
	}

	template<class T> T	Min(T x, T y)
	{
		return (x < y) ? x : y;
	}

	template<class T> INLINE T	Square(T x)
	{ 
		return x * x; 
	}

	template<class T> INLINE T	Cube(T x)   
	{ 
		return x * x * x;
	}

	template <typename Type> INLINE Type Clamp(const Type& value, const Type& min, const Type& max)
	{
		if ( value <= min )
			return min;
		else if ( value >= max )
			return max;
		else
			return value;
	}

	INLINE float LinearStep(float animStartTime, float animStopTime, float currentTime)
	{
		float anim_clip_time = Clamp(currentTime, animStartTime, animStopTime);
		return (anim_clip_time - animStartTime) / (animStopTime - animStartTime);
	}

	INLINE float Fade(float start_colour, float end_colour, float elapsed_time, float total_time)
	{
		if ( elapsed_time >= total_time )
			return end_colour;
		return start_colour + ((end_colour - start_colour) * (elapsed_time / total_time));
	}

	INLINE float LerpF(float from, float to, float time)
	{
		if ( time <= 0.0f )
			return from;
		else if ( time >= 1.0f )
			return to;
		else
			return (from + time * (to - from));
	}

	INLINE float Clamp(float time, float min, float max)
	{
		float value{ 0.0f };

		if ( time < min )
			value = min;
		else if ( time > max )
			value = max;
		else
			value = time;

		return value;
	}

	INLINE bool AreSame(float a, float b)
	{
		return fabs(a - b) < EPSILON;
	}
	
	INLINE Vec2 To_NDS(float screenWidth, float screenHeight, const Vec2& pos, bool flipY = true)
	{
		float x_norm = (1.0f / screenWidth) * pos.x;
		float y_norm = (1.0f / screenHeight) * pos.y;
		return Vec2((x_norm - 0.5f) / 0.5f,
			flipY ? -((y_norm - 0.5f) / 0.5f) : (y_norm - 0.5f) / 0.5f);
	}
	


	//------------------------------------------
	// Vector and Matrix Utils
	INLINE float Distance(const Vec2& v1, const Vec2& v2)
	{
		return sqrt(Square(v2.x - v1.x) + Square(v2.y - v1.y));
	}

	INLINE float Distance(const Vec3& v1, const Vec3& v2)
	{
		return sqrt( Square(v2.x-v1.x) + Square(v2.y - v1.y) + Square(v2.z - v1.z));
	}

	INLINE float Length(const Vec2& v)
	{
		return sqrtf(v.x*v.x + v.y*v.y);
	}

	INLINE float Length(const Vec3& v)
	{
		return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
	}

	INLINE Vec2 Snapped(const Vec2& v)
	{
		int x = static_cast<int>(v.x);
		int y = static_cast<int>(v.y);
		return Vec2(static_cast<float>(x), static_cast<float>(y));
	}
	
	INLINE Vec3 Snapped(const Vec3& v)
	{
		int x = static_cast<int>(v.x);
		int y = static_cast<int>(v.y);
		int z = static_cast<int>(v.z);
		return Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	}

	INLINE Vec2 Normalize(const Vec2& v)
	{
		float l = Length(v);
		bool z = AreSame(l, 0.0f);
		return Vec2(z ? 0.0f : v.x / l, z ? 0.0f : v.y / l);
	}

	INLINE Vec3 Normalize(const Vec3& v)
	{
		float l = Length(v);
		bool s = AreSame(l, 0.0f);
		return Vec3(s ? 0.0f : v.x / l, s ? 0.0f : v.y / l, s ? 0.0f : v.z / l);
	}

	INLINE Vec3 Vec4To3(const Vec4& v)
	{
		return Vec3(v.x, v.y, v.z);
	}

	INLINE Vec3 Up()
	{
		return Vec3(0.0f, 1.0f, 0.0f);
	}

	INLINE Vec3 Right()
	{
		return Vec3(1.0f, 0.0f, 0.0f);
	}

	INLINE Vec3 Forward()
	{
		return Vec3(0.0f, 0.0f, 1.0f);
	}

	INLINE Vec2 LerpV2(const Vec2& v1, const Vec2& v2, float time)
	{
		if ( time <= 0.0f )
			return v1;
		else if ( time >= 1.0f )
			return v2;
		else
			return Vec2(v1 + time * (v2 - v1));
	}

	INLINE Vec3 LerpV3(const Vec3& v1, const Vec3& v2, float time)
	{
		if ( time <= 0.0f )
			return v1;
		else if ( time >= 1.0f )
			return v2;
		else
			return Vec3(v1 + time * (v2 - v1));
	}

	INLINE Mat4 RotateZ(float angle)
	{
		float c = cosf(angle);
		float s = sinf(angle);
		return Mat4(
			c, -s, 0.f, 0.f,
			s, c, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f);
	}

	INLINE Mat4 RotateY(float angle)
	{
		float c = cosf(angle);
		float s = sinf(angle);
		return Mat4(
			c, 0.f, -s, 0.f,
			0.f, 1.f, 0.f, 0.f,
			s, 0.f, c, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
	}

	INLINE Mat4 RotateX(float angle)
	{
		float c = cosf(angle);
		float s = sinf(angle);
		return Mat4(
			1.f, 0.f, 0.f, 0.f,
			0.f, c, -s, 0.f,
			0.f, s, c, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
	}

	INLINE Mat4 RotateXYZ(const Vec3& euler)
	{
		return RotateX(euler.x) * RotateY(euler.y) * RotateZ(euler.z);
	}

	INLINE Mat4 Translate(const Vec3& p)
	{
		return Mat4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			p.x, p.y, p.z, 1.f);
	}

	INLINE Mat4 Scale(const Vec3& s)
	{
		return Mat4(
			s.x, 0.f, 0.f, 0.f,
			0.f, s.y, 0.f, 0.f,
			0.f, 0.f, s.z, 0.f,
			0.f, 0.f, 0.f, 1.f);
	}

	INLINE Mat4 rotate(float angle, const Vec3& v)
	{
		const float x2 = v.x * v.x;
		const float y2 = v.y * v.y;
		const float z2 = v.z * v.z;
		float rads = float(angle) * 0.0174532925f;
		const float c = cosf(rads);
		const float s = sinf(rads);
		const float omc = 1.0f - c;

		return Mat4(
			{ x2 * omc + c, v.y * v.x * omc + v.z * s, v.x * v.z * omc - v.y * s, 0 },
			{ v.x * v.y * omc - v.z * s, y2 * omc + c, v.y * v.z * omc + v.x * s, 0 },
			{ v.x * v.z * omc + v.y * s, v.y * v.z * omc - v.x * s, z2 * omc + c, 0 },
			{ 0 ,  0,  0, 1 }
		);
	}

	INLINE Mat4 frustum_perspective(float left, float right, float bottom, float top, float near, float far)
	{
		Mat4 matrix(1.0f);
		matrix[0][0] = 2 * near / (right - left);
		matrix[1][1] = 2 * near / (top - bottom);
		matrix[2][2] = -(far + near) / (far - near);
		matrix[2][3] = -1;
		matrix[3][2] = -2 * far*near / (far - near);
		matrix[2][0] = (right + left) / (right - left);
		matrix[2][1] = (top + bottom) / (top - bottom);
		matrix[3][3] = 0;
		return matrix;
	}

	INLINE Mat4 ortho_proj_transform(float l, float r, float b, float t, float n, float f)
	{
		Mat4 m;
		m[0][0] = 2.0f / (r - l);
		m[0][1] = 0.0f;         
		m[0][2] = 0.0f;        
		m[0][3] = -(r + l) / (r - l);

		m[1][0] = 0.0f;        
		m[1][1] = 2.0f / (t - b); 
		m[1][2] = 0.0f;         
		m[1][3] = -(t + b) / (t - b);
		
		m[2][0] = 0.0f;         
		m[2][1] = 0.0f;        
		m[2][2] = 2.0f / (f - n); 
		m[2][3] = -(f + n) / (f - n);

		m[3][0] = 0.0f;         
		m[3][1] = 0.0f;        
		m[3][2] = 0.0f;         
		m[3][3] = 1.0;
		return m;
	}

	/*
	void INLINE generate_normals(std::vector<Vertex>& vertices, const std::vector<dword>& elements)
	{
		for (dword i = 0; i < elements.size(); i += 3)
		{
			Vec3 a_b = vertices[elements[i + 1]].position - vertices[elements[i]].position;
			Vec3 a_c = vertices[elements[i + 2]].position - vertices[elements[i]].position;

			Vec3 n = glm::cross(a_b, a_c);

			vertices[elements[i]].normal += n;
			vertices[elements[i + 1]].normal += n;
			vertices[elements[i + 2]].normal += n;
		}

		for (dword i = 0; i < vertices.size(); ++i)
		{
			vertices[i].normal = glm::normalize(vertices[i].normal);
		}
	}

	void INLINE generate_normals(const std::vector<Vec3>& positions, const std::vector<dword>& elements, std::vector<Vec3>& normals_out)
	{
		for (dword i = 0; i < elements.size(); i += 3)
		{
			Vec3 a_b = positions[elements[i + 1]] - positions[elements[i]];
			Vec3 a_c = positions[elements[i + 2]] - positions[elements[i]];

			Vec3 n = glm::cross(a_b, a_c);

			normals_out[elements[i]] += n;
			normals_out[elements[i + 1]] += n;
			normals_out[elements[i + 2]] += n;
		}

		for (dword i = 0; i < normals_out.size(); ++i)
		{
			normals_out[i] = glm::normalize(normals_out[i]);
		}
	}
	*/
}

#endif