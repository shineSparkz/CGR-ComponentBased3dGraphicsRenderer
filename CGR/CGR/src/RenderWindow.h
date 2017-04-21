#ifndef __RENDER_WINDOW_H__
#define __RENDER_WINDOW_H__

#include "gl_headers.h"
#include <vector>
#include <tuple>

// Forward
struct GLFWwindow;

class RenderWindow
{
public:
	RenderWindow();

	bool Open(int width, int height, bool windowed, const char* title, int vsync, int aspX, int aspY, int major = 3, int minor = 3);
	int  IsOpen() const;
	void SwapBuffers();
	void SetVsync(int value);
	void Close();

private:
	friend class												Input;
	std::vector<std::tuple<const GLFWvidmode*, std::string>>	m_AvailableVideoModes;
	GLFWwindow*													m_Window;
	GLFWmonitor*												m_Monitor;

private:
	static void window_close_callback(GLFWwindow* window);
	static void window_size_callback(GLFWwindow* window, int width, int height);
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void window_pos_callback(GLFWwindow* window, int xpos, int ypos);
	static void window_iconify_callback(GLFWwindow* window, int iconified);
	static void window_focus_callback(GLFWwindow* window, int focused);
	static void window_refresh_callback(GLFWwindow* window);
};

#endif // !__RENDER_WINDOW_H__
