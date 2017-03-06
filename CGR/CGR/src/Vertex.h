#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "types.h"

struct Vertex
{
	Vec3 position;
	Vec3 normal;
	Vec2 texcoord;
};

struct VertexTan
{
	Vec3 position;
	Vec3 normal;
	Vec2 texcoord;
	Vec3 tangent;
};

#endif