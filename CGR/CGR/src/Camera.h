#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "types.h"
#include "gl_headers.h"
#include "Component.h"
#include "CamData.h"

class Transform;

class BaseCamera : public Component
{
public:
	BaseCamera(GameObject* go);
	virtual ~BaseCamera();
	
	static int GetId();

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
	void SetRange(const Vec3& range);
	void SetTarget(Transform* target);

	bool HasSkybox() const;
	SkyboxSettings* SkyBoxParams() const;

	const Vec3& Position() const;
	const Vec3& Up() const;
	const Vec3& Forward() const;
	const Vec3& Right() const;
	const Mat4& Projection() const;
	const Mat4& View() const;
	const Mat4 ProjXView() const;

	const CamType GetProjectionType() const;

protected:
	static int				m_Id;
	Transform*				m_Transform;
	Transform*				m_Target{ nullptr };
	Vec3					m_Up;
	Vec3					m_Forward;
	Vec3					m_Right;
	Vec3					m_Range;	//<-- Only used for light camera

private:
	CamType					m_CamType;
	Mat4					m_Projection;
	Mat4					m_View;
	SkyboxSettings*			m_SkyboxSettings;
	PerspectiveSettings		m_PerspectiveSettings;
};

INLINE int BaseCamera::GetId()
{
	return m_Id;
}

#endif