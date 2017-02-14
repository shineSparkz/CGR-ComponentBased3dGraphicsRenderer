#include "Event.h"
#include "LogFile.h"
#include "EventHandler.h"
#include <cassert>

Event::Event(dword id) :
	m_Listeners(),
	m_Data(nullptr),
	m_EventId(id)
{
}

Event::~Event()
{
	m_Listeners.clear();
}

void Event::Send(void* data)
{
	m_Data = data;
	std::vector<EventHandler*>::iterator i;
	for (i = m_Listeners.begin(); i != m_Listeners.end(); ++i)
	{
		if ((*i))
		{
			(*i)->HandleEvent(this);
		}
	}
}

void Event::AttachListener(EventHandler& evHandler)
{
	m_Listeners.push_back(&evHandler);
}

void Event::DetachListener(EventHandler& evHandler)
{
	std::vector<EventHandler*>::iterator i;
	for (i = m_Listeners.begin(); i != m_Listeners.end(); ++i)
	{
		if (&evHandler == *i)
		{
			m_Listeners.erase(i);
			break;
		}
	}
}

//-------------------------------------------------------------------

DelegateEvent::DelegateEvent(dword id, bool isExclusve) :
	m_Id(id)
{
	if (isExclusve)
		m_IsExclusive = GE_TRUE;
}

DelegateEvent::~DelegateEvent()
{
	m_Delegates.clear();
}

dword DelegateEvent::GetID() const
{
	return m_Id;
}

void DelegateEvent::Send(void* data)
{
	std::vector<events::Delegate>::iterator i;
	for (i = m_Delegates.begin(); i != m_Delegates.end(); ++i)
	{
		// Call each function pointer
		(*i)(data);
	}
}

bool DelegateEvent::AddDelegate(events::Delegate functor)
{
	if (m_IsExclusive == GE_TRUE)
	{
		if (m_IsExclusiveAndHasListener == GE_FALSE)
		{
			m_Delegates.push_back(functor);
			m_IsExclusiveAndHasListener = GE_TRUE;
			WRITE_LOG("Added exclusive event delegate listner", "good");
			return true;
		}
		else
		{
			// Error
			WRITE_LOG("Tried to add more than one listener to exclusive event", "error");
			return false;
		}
	}
	else
	{
		WRITE_LOG("Added event delegate listner", "good");
		m_Delegates.push_back(functor);
		return true;
	}
}

void DelegateEvent::RemoveDelegate(events::Delegate functor)
{
	std::vector<events::Delegate>::iterator i;
	for (i = m_Delegates.begin(); i != m_Delegates.end(); ++i)
	{
		if (functor == *i)
		{
			m_Delegates.erase(i);
			break;
		}
	}
}
