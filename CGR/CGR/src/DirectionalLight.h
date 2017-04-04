#ifndef __DIR_LIGHT_COMP_H__
#define __DIR_LIGHT_COMP_H__

#include "Component.h"
#include "types.h"

class UniformBlock;

class DirectionalLightC : public Component
{
public:
	DirectionalLightC(GameObject* owner);
	virtual ~DirectionalLightC();

	void Start() override;
	void Update() override;

	bool SetLight(const Vec3& direction, const Vec3& intensity);
	
	void SetDirection(const Vec3& newDir);
	void SetDirectionX(float x);
	void SetDirectionY(float x);
	void SetDirectionZ(float x);

	void SetColour(const Vec3& newCol);
	void SetColR(float r);
	void SetColG(float g);
	void SetColB(float b);

	static int GetId();

private:
	static int m_Id;
	UniformBlock* m_LightBlock;	// Weak ptr
	Vec3 m_Direction;
	Vec3 m_Intensity;
	int m_LightValid;
};

INLINE int DirectionalLightC::GetId()
{
	return m_Id;
}

#endif