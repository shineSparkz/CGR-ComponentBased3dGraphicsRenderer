#ifndef __POINT_LIGHT_COMP_H__
#define __POINT_LIGHT_COMP_H__

#include "Component.h"
#include "types.h"

class UniformBlock;

class PointLightC : public Component
{
public:
	PointLightC(GameObject* owner);
	virtual ~PointLightC();

	void Start() override;
	void Update() override;

	bool SetLight(const Vec3& position,
		const Vec3& intensity,
		float ambIntensity,
		float aConstant,
		float aLinear,
		float aQuadratic);

	void SetPosition(const Vec3& newDir);
	void SetPositionX(float x);
	void SetPositionY(float y);
	void SetPositionZ(float z);

	void SetColour(const Vec3& newCol);
	void SetColR(float r);
	void SetColG(float g);
	void SetColB(float b);

	static int GetId();

private:
	float calcPointLightBSphere();

private:
	static int		m_Id;
	UniformBlock*	m_LightBlock;	// Weak ptr
	Vec3			m_Position;
	Vec3			m_Intensity;
	float			m_AmbientIntensity;
	float			m_aConstant;
	float			m_aLinear;   
	float			m_aQuadratic;
	float			m_Range;
	int				m_LightIndex;
	int				m_LightValid;
};

INLINE int PointLightC::GetId()
{
	return m_Id;
}

#endif
