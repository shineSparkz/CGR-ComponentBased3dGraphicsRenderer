#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "types.h"
#include <string>

class BaseLight
{
public:
	std::string Name;
	Vec3 Color;
	float AmbientIntensity;
	float DiffuseIntensity;

	BaseLight()
	{
		Color = Vec3(0.0f, 0.0f, 0.0f);
		AmbientIntensity = 0.0f;
		DiffuseIntensity = 0.0f;
	}
};

class DirectionalLight : public BaseLight
{
public:
	Vec3 Direction;

	DirectionalLight()
	{
		Direction = Vec3(0.0f, 0.0f, 0.0f);
	}
};

struct LightAttenuation
{
	float Constant;
	float Linear;
	float Exp;

	LightAttenuation()
	{
		Constant = 1.0f;
		Linear = 0.0f;
		Exp = 0.0f;
	}
};

class PointLight : public BaseLight
{
public:
	Vec3 Position;
	LightAttenuation Attenuation;

	PointLight()
	{
		Position = Vec3(0.0f, 0.0f, 0.0f);
	}
};

class SpotLight : public PointLight
{
public:
	Vec3 Direction;
	float Cutoff;

	SpotLight()
	{
		Direction = Vec3(0.0f, 0.0f, 0.0f);
		Cutoff = 0.0f;
	}
};

#define COLOR_WHITE Vec3(1.0f, 1.0f, 1.0f)
#define COLOR_RED Vec3(1.0f, 0.0f, 0.0f)
#define COLOR_GREEN Vec3(0.0f, 1.0f, 0.0f)
#define COLOR_CYAN Vec3(0.0f, 1.0f, 1.0f)
#define COLOR_BLUE Vec3(0.0f, 0.0f, 1.0f)

#endif