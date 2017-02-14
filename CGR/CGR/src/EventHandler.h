#ifndef __EVENT_HANDLER_H__
#define __EVENT_HANDLER_H__

class Event;

class EventHandler
{
public:
	virtual ~EventHandler() {}
	virtual void HandleEvent(Event* ev) = 0;
};

#endif
