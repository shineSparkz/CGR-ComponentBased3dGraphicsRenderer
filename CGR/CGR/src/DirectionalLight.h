#ifndef __DIR_LIGHT_COMP_H__
#define __DIR_LIGHT_COMP_H__

#include "Component.h"
#include "types.h"

class UniformBlock;
class Renderer;

class DirectionalLightC : public Component
{
public:
	DirectionalLightC(GameObject* owner);
	virtual ~DirectionalLightC();

	void Start() override;
	void Update() override;

	bool SetLight(Renderer* renderer, const Vec3& direction, const Vec3& intensity, const Vec3& range);
	
	void SetDirection(const Vec3& newDir);
	void SetDirectionX(float x);
	void SetDirectionY(float x);
	void SetDirectionZ(float x);

	void SetRange(const Vec3& newRange);

	void SetColour(const Vec3& newCol);
	void SetColR(float r);
	void SetColG(float g);
	void SetColB(float b);

	const Vec3& GetDir() const;
	const Vec3& GetRange() const;

	static int GetId();

private:
	static int m_Id;
	Renderer*		m_Renderer;		//<-- Weak Ptr
	UniformBlock*	m_LightBlock;	//<-- Weak ptr
	Vec3			m_Direction;
	Vec3			m_Intensity;
	Vec3			m_Range;	
	int				m_LightValid;
};

INLINE int DirectionalLightC::GetId()
{
	return m_Id;
}

#endif