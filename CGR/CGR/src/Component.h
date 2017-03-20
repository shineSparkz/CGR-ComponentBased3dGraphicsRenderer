#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "types.h"
#include <string>

class GameObject;

class Component
{
public:
	Component(GameObject* owner);
	virtual ~Component();

	virtual void Start() = 0;
	virtual void Update() = 0;

	GameObject* gameObject() const;

private:
	GameObject* m_GameObject;
};

#endif