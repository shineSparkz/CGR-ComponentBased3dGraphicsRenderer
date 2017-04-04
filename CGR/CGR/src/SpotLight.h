#ifndef __SPOT_LIGHT_COMP_H__
#define __SPOT_LIGHT_COMP_H__

#include "Component.h"
#include "types.h"

class UniformBlock;

class SpotLightC : public Component
{
public:
	SpotLightC(GameObject* owner);
	virtual ~SpotLightC();

	void Start() override;
	void Update() override;

	bool SetLight(const Vec3& position,
		const Vec3& direction,
		const Vec3& intensity,
		float angle,
		float aConstant,
		float aLinear,
		float aQuadratic,
		int switchedOn
		);

	void SetPosition(const Vec3& pos);
	void SetColour(const Vec3& col);
	void SetDirection(const Vec3& dir);
	void SetAngle(float angle);
	void ToggleLight();

	static int GetId();

private:
	static int m_Id;
	UniformBlock*	m_LightBlock;	// Weak ptr
	Vec3			m_Position;
	Vec3			m_Direction;
	Vec3			m_Intensity;
	float			m_aConstant;
	float			m_aLinear;
	float			m_aQuadratic;
	float			m_CosAngle;
	int				m_SwitchedOn;
	int				m_LightIndex;
	int				m_LightValid;
};

INLINE int SpotLightC::GetId()
{
	return m_Id;
}

#endif
