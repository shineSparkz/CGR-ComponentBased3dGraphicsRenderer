#include "RenderWindow.h"

#include <string>
#include <iostream>

#include "Input.h"
#include "LogFile.h"
#include "Screen.h"
#include "math_utils.h"
#include "utils.h"
#include "Renderer.h"
#include "EventManager.h"

RenderWindow::RenderWindow() :
	m_Window(nullptr),
	m_Monitor(nullptr)
{
}

bool RenderWindow::Open(int width, int height, bool windowed, const char* title, int vsync, int aspX, int aspY, int major, int minor)
{
	// Re-create if already exists
	if (this->m_Window)
		glfwDestroyWindow(glfwGetCurrentContext());

	m_Monitor = glfwGetPrimaryMonitor();

	// Log monitor
	std::string monitor_str = "Monitor: " + (std::string)glfwGetMonitorName(m_Monitor);
	WRITE_LOG(monitor_str, "none");

	// TODO : configurable resolutions, Get highest resolution for now
	int modes_count = 0;
	const GLFWvidmode* modes = glfwGetVideoModes(m_Monitor, &modes_count);
	for (size_t i = 0; i < (size_t)modes_count - 1; ++i)
	{
		++modes;
		if (modes)
		{
			std::string av_mode = "Resolution :" + std::to_string(modes->width) +
				"x" + std::to_string(modes->height) + "  " + std::to_string(modes->refreshRate) + "hz";
			
			//WRITE_LOG(av_mode, "none");

			m_AvailableVideoModes.push_back(std::make_tuple(modes, av_mode));
		}
	}

	// Set window hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	glfwWindowHint(GLFW_RED_BITS, modes->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, modes->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, modes->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, modes->refreshRate);
	// TODO : More hints

	this->m_Window = glfwCreateWindow(
		(int)Maths::Clamp((float)width, 640.0f, (float)modes->width),
		(int)Maths::Clamp((float)height, 480.0f, (float)modes->height),
		title,
		windowed ? NULL : glfwGetPrimaryMonitor(),
		NULL
	);

	if (!m_Window)
	{
		WRITE_LOG("Window not created", "error");
		return false;
	}

	// TODO : add a bool to init to see if the window should be clamped
	glfwSetWindowAspectRatio(m_Window, aspX, aspY);

	// -----------------------  Call backs ---------------------------------
	glfwSetWindowCloseCallback(m_Window, RenderWindow::window_close_callback);
	glfwSetWindowSizeCallback(m_Window, RenderWindow::window_size_callback);
	glfwSetFramebufferSizeCallback(m_Window, RenderWindow::framebuffer_size_callback);
	glfwSetWindowPosCallback(m_Window, RenderWindow::window_pos_callback);
	glfwSetWindowIconifyCallback(m_Window, RenderWindow::window_iconify_callback);
	glfwSetWindowFocusCallback(m_Window, RenderWindow::window_focus_callback);
	glfwSetWindowRefreshCallback(m_Window, RenderWindow::window_refresh_callback);


	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwMakeContextCurrent(m_Window);

	//------- Log window params-------------------------------------------
	Screen::m_FullScreen = !windowed;
	Screen::m_VideoModeHandle = m_AvailableVideoModes.size() - 1; // TODO : config

	glfwGetWindowSize(m_Window, &Screen::m_ScreenWidth, &Screen::m_ScreenHeight);
	glfwGetFramebufferSize(m_Window, &Screen::m_FrameBuffWidth, &Screen::m_FrameBuffHeight);
	
	return true;
}

int RenderWindow::IsOpen() const
{
	return !glfwWindowShouldClose(m_Window);
}

void RenderWindow::SetVsync(int value)
{
	glfwSwapInterval(Maths::Clamp(value, 0, 1));
}

void RenderWindow::SwapBuffers()
{
	glfwSwapBuffers(m_Window);
}

void RenderWindow::Close()
{
	glfwDestroyWindow(m_Window);
}

//-----  Window Callbacks----------------------------
void RenderWindow::window_close_callback(GLFWwindow* window)
{
	WRITE_LOG("window close callback: closing", "normal");
	// the window has already been closed here, so everything should be cleaned up
}

void RenderWindow::window_size_callback(GLFWwindow* window, int width, int height)
{
	Screen::m_ScreenWidth = width;
	Screen::m_ScreenHeight = height;

	Vec2 screen_params((float)width, (float)height);
	EventManager::Instance()->SendEvent(EVENT_WINDOW_SIZE_CHANGE, &screen_params);
	WRITE_LOG("window size callback: width:" + util::to_str(width) + " height:" + util::to_str(height), "normal");
}

void RenderWindow::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	Screen::m_FrameBuffWidth = width;
	Screen::m_FrameBuffHeight = height;
	glViewport(0, 0, width, height);
}

void RenderWindow::window_pos_callback(GLFWwindow* window, int xpos, int ypos)
{
}

void RenderWindow::window_iconify_callback(GLFWwindow* window, int iconified)
{
	WRITE_LOG("window iconify callback: iconified:" + util::to_str(iconified), "normal");

	if (iconified)
	{
		// The window was iconified
	}
	else
	{
		// The window was restored
	}
}

void RenderWindow::window_focus_callback(GLFWwindow* window, int focused)
{
	WRITE_LOG("window iconify callback: focused:" + util::to_str(focused), "normal");

	int focusParam = focused;
	EventManager::Instance()->SendEvent(EVENT_WINDOW_FOCUS, &focusParam);

	/*
	if (focused)
	{
		// The window gained input focus
	}
	else
	{
		// The window lost input focus
	}
	*/
}

void RenderWindow::window_refresh_callback(GLFWwindow* window)
{
	WRITE_LOG("window refresh callback", "normal");
	glfwSwapBuffers(window);
}