#include "Screen.h"

int		Screen::m_ScreenWidth = 0;
int		Screen::m_ScreenHeight = 0;
int		Screen::m_FrameBuffWidth = 0;
int		Screen::m_FrameBuffHeight = 0;
bool	Screen::m_FullScreen = 0;
size_t	Screen::m_VideoModeHandle = 0;

Screen::Screen()
{
}