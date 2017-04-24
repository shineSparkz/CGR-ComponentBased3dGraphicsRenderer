#ifndef __CHASE_CAM_H__
#define __CHASE_CAM_H__

#include "Camera.h"

class Transform;

class ChaseCamera2D : public BaseCamera
{
public:
	ChaseCamera2D(GameObject* go);
	virtual ~ChaseCamera2D();

	void Start() override;
	void Update() override;
};

#endif