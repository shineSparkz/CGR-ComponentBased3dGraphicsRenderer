#ifndef __FPS_CAMERA_H__
#define __FPS_CAMERA_H__

#include "Camera.h"
#include "EventHandler.h"

class TerrainConstructor;

struct CollisionPacket
{
	// Info about elip in world
	Vec3 ellipsoidSpace;
	Vec3 w_pos;
	Vec3 w_vel;

	// Elip space
	Vec3 e_pos;
	Vec3 e_vel;
	Vec3 e_norm_vel;

	bool found_collision;
	float nearest_distance;
	Vec3 intersection_point;
	int collision_recursion_depth;
};

class FpsCamera : public EventHandler, public BaseCamera
{
public:
	FpsCamera(GameObject* go);
	virtual ~FpsCamera();

	void Start() override;
	void Update() override;

	void SetTerrain(TerrainConstructor* t)
	{
		m_Terrain = t;
	}

private:
	void HandleEvent(Event* ev) override;

private:
	TerrainConstructor*		m_Terrain;		// <-- WeakPtr
	Vec3					m_Velocity;
	Vec3					m_Previous;
	float					m_MoveSpeed;
	float					m_MouseSpeed;
	int						m_WindowFocused;
	bool					m_Fkey, m_Bkey, m_Lkey, m_Rkey;
};

#endif
