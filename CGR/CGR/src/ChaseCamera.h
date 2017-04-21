#ifndef __CHASE_CAM_H__
#define __CHASE_CAM_H__

#include "Camera.h"
#include "EventHandler.h"

class ChaseCamera2D : public EventHandler, public BaseCamera
{
public:
	ChaseCamera2D(GameObject* go);
	virtual ~ChaseCamera2D();

	void Start() override;
	void Update() override;

private:
	void HandleEvent(Event* ev) override;

private:
	Vec3 m_Velocity;
	float m_MoveSpeed;
	bool m_Fkey, m_Bkey, m_Lkey, m_Rkey;
};

#endif