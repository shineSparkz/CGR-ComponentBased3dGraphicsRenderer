#include "EventManager.h"
#include "Application.h"
#include "LogFile.h"

EventManager::EventManager() 
{
}

EventManager::~EventManager()
{
	for (EventMapIter iter = m_EventMap.begin(); iter != m_EventMap.end(); ++iter)
	{
		Event* pEvent = iter->second;
		if (pEvent)
		{
			SAFE_DELETE(pEvent);
		}
	}

	for (DelEventIter it = m_DelEvents.begin(); it != m_DelEvents.end(); ++it)
	{
		DelegateEvent* e = it->second;
		if (e)
		{
			SAFE_DELETE(e);
		}
	}

	m_EventMap.clear();
	m_DelEvents.clear();
}

void EventManager::SendDelEvent(dword eventId, void* pData)
{
	DelEventIter result = m_DelEvents.find(eventId);
	if (result != m_DelEvents.end())
	{
		if (result->second)
		{
			result->second->Send(pData);
		}
	}
}

bool EventManager::RegisterDelEvent(dword eventId, bool exclusive)
{
	// Prevent the user of this API trying to register events that we want
	if (!Application::Instance())
	{
		// Can't log here because no app, then no logger
		return false;
	}

	bool added = false;

	DelEventIter result = m_DelEvents.find(eventId);
	if (result == m_DelEvents.end())
	{
		DelegateEvent* pNewEvent = new DelegateEvent(eventId, exclusive);

		if (pNewEvent)
		{
			std::pair<dword, DelegateEvent*> newEvent(eventId, pNewEvent);
			std::pair<DelEventIter, bool> addedIter = m_DelEvents.insert(newEvent);
			added = addedIter.second;
		}
	}

	return added;
}

bool EventManager::IsDelegateRegistered(dword id)
{
	DelEventIter result = m_DelEvents.find(id);
	return result != m_DelEvents.end();
}

bool EventManager::AttachDelegate(dword eventId, events::Delegate functor)
{
	DelEventIter result = m_DelEvents.find(eventId);

	if (result != m_DelEvents.end())
	{
		return result->second->AddDelegate(functor);
	}
	else
	{
		std::string err = "Tried to attach the event delegate : " + std::to_string((int)eventId) + " that does not exist.";
		WRITE_LOG(err, "warning");
		return false;
	}
}

void EventManager::RemoveDelegate(dword eventId, events::Delegate functor)
{
	DelEventIter result = m_DelEvents.find(eventId);
	if (result != m_DelEvents.end())
	{
		result->second->RemoveDelegate(functor);
	}
	else
	{
		std::string err = "Can not detach event : " + std::to_string((int)eventId) + " because it doesn't exist.";
		WRITE_LOG(err, "warning");
	}
}

//---------------------------------------------------------------------------------------

void EventManager::SendEvent(dword id, void* data)
{
	EventMapIter result = m_EventMap.find(id);
	if (result != m_EventMap.end())
	{
		//assert(result->second);
		if (result->second)
		{
			result->second->Send(data);
		}
	}
}

bool EventManager::RegisterEvent(dword id)
{
	bool added = false;

	EventMapIter result = m_EventMap.find(id);
	if (result == m_EventMap.end())
	{
		Event* pNewEvent = new Event(id);

		if (pNewEvent)
		{
			std::pair<dword, Event*> newEvent(id, pNewEvent);
			std::pair<EventMapIter, bool> addedIter = m_EventMap.insert(newEvent);
			added = addedIter.second;
		}
	}

	return added;
}

bool EventManager::AttachEvent(dword id, EventHandler& ev)
{
	EventMapIter result = m_EventMap.find(id);

	if (result != m_EventMap.end())
	{
		result->second->AttachListener(ev);
		return true;
	}
	else
	{
		std::string err = "Tried to attach the game event : " + std::to_string(id) + " that does not exist.";
		WRITE_LOG(err, "warning");
		return false;
	}
}

void EventManager::RemoveEvent(dword id, EventHandler& ev)
{
	EventMapIter result = m_EventMap.find(id);
	if (result != m_EventMap.end())
	{
		result->second->DetachListener(ev);
	}
	else
	{
		std::string err = "Can not detach event : " + std::to_string(id) + " because it doesn't exist.";
		WRITE_LOG(err, "warning");
	}
}

bool EventManager::IsEventRegistered(dword id)
{
	EventMapIter result = m_EventMap.find(id);
	return result != m_EventMap.end();
}