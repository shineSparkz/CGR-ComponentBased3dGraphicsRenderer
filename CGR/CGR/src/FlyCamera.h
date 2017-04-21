#ifndef __FLY_CAMERA_H__
#define __FLY_CAMERA_H__

#include "Camera.h"
#include "EventHandler.h"

class FlyCamera : public EventHandler, public BaseCamera
{
public:
	FlyCamera(GameObject* go);
	virtual ~FlyCamera();

	void Start() override;
	void Update() override;

	void SetSpeeds(float moveSpeed, float mouseSpeed);

private:
	void HandleEvent(Event* ev) override;

private:
	Vec3 m_Velocity;
	float m_MoveSpeed;
	float m_MouseSpeed;
	int m_WindowFocused;
	bool m_Fkey, m_Bkey, m_Lkey, m_Rkey;
};

#endif