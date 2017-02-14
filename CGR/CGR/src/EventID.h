#ifndef __EVENT_ID_H__
#define __EVENT_ID_H__

#define KEY_EVENT			0
#define WINDOW_FOCUS_EVENT	1
#define SHUTDOWN_EVENT		2
#define NUM_ENGINE_EVENTS	3

namespace events
{
	typedef void(*Delegate)(void* data);
}

#endif