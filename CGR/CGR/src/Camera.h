#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "types.h"
#include "gl_headers.h"
#include "EventHandler.h"
#include "Component.h"

class Transform;

enum CamType
{
	Perspective,
	Orthographic,
	Shadow
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

class BaseCamera : public Component
{
public:
	BaseCamera(GameObject* go);
	static int GetId();
	virtual ~BaseCamera();

	void Start() override;
	void Update() override;

	void Init(CamType type, const Vec3& position, const Vec3& up, const Vec3& right, const Vec3& forward, float fov, float aspect, float near, float far);
	void AddSkybox(float scale, unsigned cubeMapIndex);

	void SetAspect(float asp);
	void SetNear(float near);
	void SetFar(float far);
	void SetFOV(float fov);
	void SetPosition(const Vec3& p);
	void SetDirection(const Vec3& p);
	void SetUp(const Vec3& u);

	bool HasSkybox() const;
	SkyboxSettings* SkyBoxParams() const;

	const Vec3& Position() const;
	const Vec3& Up() const;
	const Vec3& Forward() const;
	const Vec3& Right() const;
	const Mat4& Projection() const;
	const Mat4& View() const;
	const Mat4 ProjXView() const;

	const CamType GetProjectionType() const
	{
		return camType;
	}

protected:
	static int m_Id;
	Transform* m_Transform;
	Vec3 up;
	Vec3 forward;
	Vec3 right;

private:
	CamType camType;
	Mat4 projection;
	Mat4 view;
	SkyboxSettings* skyboxSettings;
	PerspectiveSettings perspectiveSettings;
};

INLINE int BaseCamera::GetId()
{
	return m_Id;
}

class FlyCamera : public EventHandler, public BaseCamera
{
public:
	FlyCamera(GameObject* go);
	virtual ~FlyCamera();

	void Start() override;
	void Update() override;

private:
	void HandleEvent(Event* ev) override;

private:
	Vec3 velocity;
	bool fk, bk, lk, rk;
	float speed = 45.0f;
	float mouseSpeed = 0.4f;
	int windowFocused = 1;
};

class ChaseCamera2D : public EventHandler, public BaseCamera
{
public:
	ChaseCamera2D(GameObject* go);
	virtual ~ChaseCamera2D();

	void Start() override;
	void Update() override;

private:
	void HandleEvent(Event* ev) override;

	Vec3 velocity;
	bool fk, bk, lk, rk;
	float speed = 565.0f;
	float mouseSpeed = 0.04f;
};

#endif