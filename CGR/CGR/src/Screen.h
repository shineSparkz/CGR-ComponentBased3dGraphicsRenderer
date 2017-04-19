#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "types.h"

class Screen
{
public:
	Screen();

	static int		ScreenWidth();
	static int		ScreenHeight();
	static int		FrameBufferWidth();
	static int		FrameBufferHeight();
	static bool		IsFullScreen();

	// TODO Later : screen resolution stuff

private:
	friend class	RenderWindow;
	static int		m_ScreenWidth;
	static int		m_ScreenHeight;
	static int		m_FrameBuffWidth;
	static int		m_FrameBuffHeight;
	static size_t	m_VideoModeHandle;
	static bool		m_FullScreen;
};

INLINE int Screen::ScreenWidth()
{
	return m_ScreenWidth;
}

INLINE int Screen::ScreenHeight()
{
	return m_ScreenHeight;
}

INLINE int Screen::FrameBufferWidth()
{
	return m_FrameBuffWidth;
}

INLINE int Screen::FrameBufferHeight()
{
	return m_FrameBuffHeight;
}

INLINE bool Screen::IsFullScreen()
{
	return m_FullScreen;
}
#endif