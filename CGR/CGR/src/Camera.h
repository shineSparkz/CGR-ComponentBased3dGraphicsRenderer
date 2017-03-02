#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "types.h"
#include "gl_headers.h"

//temp as fuck
#include "EventHandler.h"

class Camera : public EventHandler
{
public:
	Camera();

	Vec3 position;
	Vec3 up;
	Vec3 forward;
	Vec3 right;

	Mat4 projection;
	Mat4 view;

	float fov;
	float aspect;
	float near;
	float far;

	void Update();

private:
	void HandleEvent(Event* ev) override;

	Vec3 velocity;
	bool fk, bk, lk, rk;
	float yaw = 0.0f;
	float pitch = 0.0f;
	float speed = 30.0f;
	float mouseSpeed = 1.4f;
};

#endif