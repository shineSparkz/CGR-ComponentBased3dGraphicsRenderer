#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__

#include <unordered_map>
#include "Singleton.h"
#include "Event.h"

class EventManager : public Singleton<EventManager>
{
public:
	EventManager();
	~EventManager();

	void SendDelEvent(dword id, void* data);
	bool RegisterDelEvent(dword id, bool exclusive = false);
	bool AttachDelegate(dword id, events::Delegate);
	void RemoveDelegate(dword id, events::Delegate);
	bool IsDelegateRegistered(dword id);

	void SendEvent(dword id, void* data);
	bool RegisterEvent(dword id);
	bool AttachEvent(dword id, EventHandler& ev);
	void RemoveEvent(dword id, EventHandler& ev);
	bool IsEventRegistered(dword id);

private:
	typedef std::unordered_map<dword, Event*> EventMap;
	typedef EventMap::iterator EventMapIter;
	typedef std::unordered_map<dword, DelegateEvent* > DelegateEventMap;
	typedef DelegateEventMap::iterator DelEventIter;

	EventMap m_EventMap;
	DelegateEventMap m_DelEvents;
};

#endif
