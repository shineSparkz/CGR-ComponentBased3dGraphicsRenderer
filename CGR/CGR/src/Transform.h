#ifndef  __TRANSFORM_H__
#define  __TRANSFORM_H__

#include "Component.h"
#include "types.h"
#include "gl_headers.h"

class Transform : public Component
{
public:
	Transform(GameObject* owner);
	virtual ~Transform();

	static int GetId();

	void Start() override;
	void Update() override;

	void UseQuatsForRotation(bool use);

	void SetPosition(const Vec3& p);
	void SetScale(const Vec3& p);
	void MovePosition(const Vec3& p);

	void RotateX(float angle);
	void RotateY(float angle);
	void RotateZ(float angle);
	void Rotate(float x, float y, float z);
	void Rotate(const Vec3& a);

	const Mat4& GetModelXform() const;
	const Vec3& Position() const;
	const Vec3& Euler() const;
	const Vec3& Scale() const;

private:
	static int m_Id;
	Mat4 model_xform;
	Vec3 position;
	Vec3 scale;
	Vec3 euler;
	bool use_quats = false;
};

INLINE int Transform::GetId()
{
	return m_Id;
}

INLINE void Transform::MovePosition(const Vec3& p)
{
	this->position += p;
}

INLINE const Mat4& Transform::GetModelXform() const
{
	return model_xform;
}

INLINE const Vec3& Transform::Position() const
{
	return position;
}

INLINE const Vec3& Transform::Euler() const
{
	return euler;
}

INLINE const Vec3& Transform::Scale() const
{
	return scale;
}

INLINE void Transform::UseQuatsForRotation(bool use)
{
	this->use_quats = use;
}

INLINE void Transform::RotateX(float angle)
{
	euler.x += angle;
}

INLINE void Transform::RotateY(float angle)
{
	euler.y += angle;
}

INLINE void Transform::RotateZ(float angle)
{
	euler.z += angle;
}

INLINE void Transform::Rotate(float x, float y, float z)
{
	euler = Vec3(x, y, x);
}

INLINE void Transform::Rotate(const Vec3& a)
{
	euler = a;
}

#endif // ! __TRANSFORM_H__
