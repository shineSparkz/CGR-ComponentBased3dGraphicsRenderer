#ifndef __CAM_DATA_H__
#define __CAM_DATA_H__

enum CamType
{
	Perspective,
	Orthographic,
	Light
};

struct PerspectiveSettings
{
	PerspectiveSettings() :
		fov(0.0f),
		aspect(0.0f),
		near(0.0f),
		far(0.0f)
	{
	}

	PerspectiveSettings(float fov_, float aspect_, float near_, float far_) :
		fov(fov_),
		aspect(aspect_),
		near(near_),
		far(far_)
	{
	}

	float fov;
	float aspect;
	float near;
	float far;
};

struct SkyboxSettings
{
	float scale = 0.0f;
	unsigned textureIndex;
};

#endif