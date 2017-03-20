#include "GameObject.h"
#include "Component.h"

GameObject::GameObject() :
	m_Components(),
	m_Enabled(true)
{
}

GameObject::~GameObject()
{
}

void GameObject::Close()
{
	for (CompIter i = m_Components.begin(); i != m_Components.end(); ++i)
	{
		Component* c = i->second;
		if (c)
		{
			SAFE_DELETE(c);
		}
	}

	m_Components.clear();
}

void GameObject::Start()
{
	for (CompIter i = m_Components.begin(); i != m_Components.end(); ++i)
	{
		i->second->Start();
	}
}

void GameObject::Update()
{
	if (this->m_Enabled)
	{
		for (CompIter i = m_Components.begin(); i != m_Components.end(); ++i)
		{
			i->second->Update();
		}
	}
}

