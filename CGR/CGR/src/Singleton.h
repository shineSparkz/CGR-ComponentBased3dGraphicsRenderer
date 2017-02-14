#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <cassert>

template<typename T>
class Singleton
{
public:
	Singleton()
	{
		assert(!m_Instance);
		m_Instance = static_cast<T*>(this);
	}

	~Singleton()
	{
		assert(m_Instance);
		m_Instance = 0;
	}

	static T* Instance() { return m_Instance; }

private:
	static T* m_Instance;
};

template<typename T>
T* Singleton<T>::m_Instance;

#endif