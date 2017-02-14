#ifndef __EVENT_H__
#define __EVENT_H__

#include <vector>
#include "EventID.h"
#include "types.h"

class EventHandler;

class Event
{
public:
	Event(dword id);
	virtual ~Event();

	dword GetID() const;
	void* GetData() const;

	void Send(void* data);
	void AttachListener(EventHandler& evHandler);
	void DetachListener(EventHandler& evHandler);

private:
	std::vector<EventHandler*> m_Listeners;
	void* m_Data{ nullptr };
	dword m_EventId{ 0 };
};

class DelegateEvent
{
public:
	DelegateEvent(dword id, bool isExclusive = false);
	~DelegateEvent();

	dword GetID() const;
	void Send(void* data);

	bool AddDelegate(events::Delegate functor);
	void RemoveDelegate(events::Delegate functor);

private:
	std::vector<events::Delegate> m_Delegates;
	dword m_Id;
	word m_IsExclusive{ 0 };
	word m_IsExclusiveAndHasListener{ 0 };
};

INLINE dword Event::GetID() const
{
	return m_EventId;
}

INLINE void* Event::GetData() const
{
	return m_Data;
}

#endif