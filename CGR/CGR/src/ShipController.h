#ifndef __SHIP_CONTROLLER_H__
#define __SHIP_CONTROLLER_H__

#include "Component.h"

class Transform;

class ShipController : public Component
{
public:
	ShipController(GameObject* owner);
	virtual ~ShipController();

	static int GetId();

	void SetCam(Transform* t);

	void Start() override;
	void Update() override;

private:
	static int m_Id;
	Transform*  m_Transform;
	Transform*	m_CamTransform;
};

INLINE int ShipController::GetId()
{
	return m_Id;
}

#endif