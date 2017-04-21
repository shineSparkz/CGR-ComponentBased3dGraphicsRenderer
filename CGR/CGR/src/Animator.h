#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__

#include "anim_types.h"
#include "Component.h"

class Animator : public Component
{
public:
	Animator(GameObject* go);
	virtual ~Animator();

	static int GetId();

	void Start() override;
	void Update() override;

	void StartAnimation(animType_t anim);

private:
	friend class Renderer;
	static int	m_Id;
	animState_t	m_AnimState;
};

INLINE int Animator::GetId()
{
	return m_Id;
}

#endif