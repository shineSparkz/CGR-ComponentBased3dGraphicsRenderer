#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include "types.h"
#include <unordered_map>
#include <iostream>
#include <string>

class Component;

class GameObject
{
	typedef std::unordered_map<int, Component*> ComponentHash;
	typedef ComponentHash::iterator CompIter;

public:
	GameObject();
	~GameObject();

	template<class T>
	T* AddComponent();

	void Start();
	void Update();
	void Close();

	template <class T>
	T*	GetComponent() { return static_cast<T*>(GetComponent(T::GetId())); }

	void SetActive(bool enabled);
	bool Enabled() const;

private:
	Component* GetComponent(int type);

private:
	ComponentHash m_Components;
	bool m_Enabled;

};

template <class T>
INLINE T* GameObject::AddComponent()
{
	CompIter result = m_Components.find(T::GetId());
	if (result == m_Components.end())
	{
		T* new_component = new T(this);

		if (new_component)
		{
			std::pair<int, Component*> component(T::GetId(), new_component);
			m_Components.insert(component);
		}

		return new_component;
	}

	return nullptr;
}

INLINE Component* GameObject::GetComponent(int type)
{
	CompIter result = m_Components.find(type);
	return result == m_Components.end() ?
		nullptr : result->second;
}

INLINE bool GameObject::Enabled() const
{
	return this->m_Enabled;
}

INLINE void GameObject::SetActive(bool enabled)
{
	this->m_Enabled = enabled;
}

#endif