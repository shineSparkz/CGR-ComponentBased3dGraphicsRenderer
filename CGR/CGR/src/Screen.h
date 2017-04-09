#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "Singleton.h"
#include "types.h"

class Screen : public Singleton<Screen>
{
public:
	Screen() {}

	int		ScreenWidth()		const;
	int		ScreenHeight()		const;
	int		FrameBufferWidth()	const;
	int		FrameBufferHeight() const;
	bool	IsFullScreen()		const;

	// TODO Later : screen resolution stuff

private:
	friend class	RenderWindow;
	int				m_ScreenWidth{ 0 };
	int				m_ScreenHeight{ 0 };
	int				m_FrameBuffWidth{ 0 };
	int				m_FrameBuffHeight{ 0 };
	size_t			m_VideoModeHandle{ 0 };
	bool			m_FullScreen{ 0 };
};

INLINE int Screen::ScreenWidth() const
{
	return m_ScreenWidth;
}

INLINE int Screen::ScreenHeight() const
{
	return m_ScreenHeight;
}

INLINE int Screen::FrameBufferWidth() const
{
	return m_FrameBuffWidth;
}

INLINE int Screen::FrameBufferHeight() const
{
	return m_FrameBuffHeight;
}

INLINE bool Screen::IsFullScreen() const
{
	return m_FullScreen;
}
#endif