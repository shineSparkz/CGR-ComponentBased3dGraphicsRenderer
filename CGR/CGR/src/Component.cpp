#include "Component.h"

Component::Component(GameObject* owner) :
	m_GameObject(owner)
{
}

Component::~Component()
{
}

GameObject* Component::gameObject() const
{
	return m_GameObject;
}