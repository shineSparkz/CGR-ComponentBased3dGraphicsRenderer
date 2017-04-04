#ifndef __TYPES_H__
#define __TYPES_H__

#include <glm/glm.hpp>

#ifdef WIN32
#define SNPRINTF _snprintf_s
#else
#define SNPRINTF snprintf
#endif

#define INLINE inline

#define GE_TRUE		1
#define GE_FALSE	0

#define GE_OK			0x00000000
#define GE_FATAL_ERROR	0x00000001
#define GE_MAJOR_ERROR	0x00000002
#define GE_MINOR_ERROR	0X00000003

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

// ------- MACROS ---------------
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p) = nullptr; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p) = nullptr; } }
#endif
#ifndef SAFE_CLOSE
#define SAFE_CLOSE(p)      { if (p) { (p)->Close();   delete (p);   (p) = nullptr; } }
#endif

#define MAX_TYPE( x )			( ( ( ( 1 << ( ( sizeof( x ) - 1 ) * 8 - 1 ) ) - 1 ) << 8 ) | 255 )
#define MIN_TYPE( x )			( - MAX_TYPE( x ) - 1 )
#define MAX_UNSIGNED_TYPE( x )	( ( ( ( 1U << ( ( sizeof( x ) - 1 ) * 8 ) ) - 1 ) << 8 ) | 255U )
#define MIN_UNSIGNED_TYPE( x )	0

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define INVALID_UNIFORM_LOCATION 0xffffffff
#define INVALID_TEXTURE_LOCATION 0xfffffffe


typedef glm::vec3 Vec3;
typedef glm::vec2 Vec2;
typedef glm::vec4 Vec4;
typedef glm::mat4 Mat4;
typedef glm::mat3x4 Mat3x4;
typedef glm::quat Quat;

typedef unsigned int		dword;
typedef unsigned char		byte;
typedef unsigned short		word;
typedef int					int32;
typedef unsigned int		uint32;
typedef long long			int64;
typedef unsigned long long	uint64;

// Negative values are engine specific
#define TRANSFORM_COMPONENT			-1
#define MESH_RENDER_COMPONENT		-2
#define CAMERA_COMPONENT			-3
#define DIRECTION_LIGHT_COMPONENT	-4
#define POINT_LIGHT_COMPONENT		-5
#define SPOT_LIGHT_COMPONENT		-6

// Transforms -- for now
/*
const Mat4 MALE_XFORM =  glm::translate(Mat4(1.0f), Vec3(-0.5f, -1.5f, -10.0f))  *  glm::scale(Mat4(1.0f), Vec3(1.0f));
const Mat4 DINO_XFORM =  glm::translate(Mat4(1.0f), Vec3(1.8f, -1.5f, -10.0f))   *  glm::scale(Mat4(1.0f), Vec3(0.05f));
const Mat4 CUBE1_XFORM = glm::translate(Mat4(1.0f), Vec3(0.0f, 1.0f, -10.0f))   *  glm::scale(Mat4(1.0f), Vec3(1.0f));
const Mat4 CUBE2_XFORM = glm::translate(Mat4(1.0f), Vec3(-2.0f, 0.0f, 0.0f))   *  glm::scale(Mat4(1.0f), Vec3(0.5f));
const Mat4 FLOOR_XFORM = glm::translate(Mat4(1.0f), Vec3(0.0f, -2.0f, -3.0f))  *  glm::scale(Mat4(1.0f), Vec3(30.0f, 0.5f, 30.0f));
const Mat4 LAVA_XFORM = glm::translate(Mat4(1.0f),  Vec3(0.0f, 4.0f, 0.0f)) *  glm::scale(Mat4(1.0f), Vec3(400.0f, 2.0f, 400.0f));
*/

#endif