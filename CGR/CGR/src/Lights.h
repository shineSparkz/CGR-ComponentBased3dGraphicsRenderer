#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "types.h"
#include <string>

struct DirectionalLight
{
	Vec3 direction;
	Vec3 intensity;
	float ambient_intensity;

	DirectionalLight() :
		direction(0.0f),
		intensity(0.0f),
		ambient_intensity(0.0f)
	{
	}

	DirectionalLight(const Vec3& direction_ , const Vec3& intensity_, float ambient_intensity_) :
		direction(direction_),
		intensity(intensity_),
		ambient_intensity(ambient_intensity_)
	{
	}
};

struct PointLight
{
	Vec3    position;   
	Vec3    intensity;  //!< The colour and brightness of the light.
	float   ambient_intensity;
	float   aConstant;  //!< The constant co-efficient for the attenuation formula.
	float   aLinear;    //!< The linear co-efficient for the attenuation formula.
	float   aQuadratic; //!< The quadratic co-efficient for the attenuation formula.

	PointLight()
	{
		position = Vec3(0.0f);
		intensity = Vec3(0.0f);
		ambient_intensity = 0.0f;
		aConstant = 0.0f;
		aLinear = 0.0f;
		aQuadratic = 0.0f;
	}

	PointLight(const Vec3& pos, const Vec3& col, float ambient, float aCon, float aLin, float aQuad) :
		position(pos),
		intensity(col),
		ambient_intensity(ambient),
		aConstant(aCon),
		aLinear(aLin),
		aQuadratic(aQuad)
	{
	}
};

struct SpotLight
{
	Vec3 position;
	Vec3 direction;
	Vec3 intensity;
	float aConstant;
	float aLinear;
	float aQuadratic;
	int switched_on;

	SpotLight()
	{
		position = Vec3(0.0f);
		direction = Vec3(0.0f);
		intensity = Vec3(0.0f);
		cosAngle = 0.0f;
		aConstant = 0.0f;
		aLinear = 0.0f;
		aQuadratic = 0.0f;
		switched_on = 1;
	}

	SpotLight(const Vec3& pos, const Vec3& dir, const Vec3& col, float coneAngle, float aCon, float aLin, float aQuad, int on) :
		position(pos),
		direction(dir),
		intensity(col),
		cosAngle(cosf(coneAngle*3.1415f / 180.0f)),
		aConstant(aCon),
		aLinear(aLin),
		aQuadratic(aQuad),
		switched_on(on)
	{
	}

	void SetAngle(float coneAngle)
	{
		this->cosAngle = cosf(coneAngle*3.1415f / 180.0f);
	}

	const float& GetAngle() const
	{
		return cosAngle;
	}

	void ToggleLight()
	{
		if (switched_on > 0)
			switched_on = 0;
		else
			switched_on = 1;
	}

private:
	float cosAngle;
};

#endif