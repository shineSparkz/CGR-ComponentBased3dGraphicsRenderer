#ifndef __CGR_ENGINE_H__
#define __CGR_ENGINE_H__

#include "types.h"

class GameObject;
class Renderer;

class CgrEngine
{
public:
	static GameObject* CreatePointLight(Renderer* renderer, const Vec3& position, const Vec3& colour, float intensity);
	static GameObject* CreateSpotLight(Renderer* renderer, const Vec3& position, const Vec3& colour, const Vec3& direction, float angle, int switchedOn);
};

#endif
