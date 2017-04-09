#ifndef __EVENT_ID_H__
#define __EVENT_ID_H__

#define EVENT_KEY						0
#define EVENT_WINDOW_FOCUS				1
#define EVENT_SHUTDOWN					2
#define EVENT_WINDOW_SIZE_CHANGE		3
#define EVENT_SCENE_CHANGE				4
#define NUM_ENGINE_EVENTS				5

namespace events
{
	typedef void(*Delegate)(void* data);
}

#endif