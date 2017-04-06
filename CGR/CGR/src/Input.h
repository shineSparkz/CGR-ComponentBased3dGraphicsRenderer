#ifndef __INPUT_H__
#define __INPUT_H__

#include "Time.h"
#include "Singleton.h"
#include "gl_headers.h"
#include "types.h"
#include <map>

class RenderWindow;
struct GLFWwindow;

class Input;

class Mouse : public Singleton<Mouse>
{
public:
	Mouse(Input* input);

	double PosX() const;
	double PosY() const;

	void GetMousePosition(double& x, double& y);
	void SetMousePosition(double x, double y);

	bool LMB = false;
	bool RMB = false;

private:
	friend class Input;
	Input* m_InputManager;
	double m_Xpos;
	double m_Ypos;
};

class Input
{
public:
	~Input();

	int Init();

	static std::map<int, int> Keys;

private:
	void GetMousePosition(double& x, double& y);
	void SetMousePosition(double x, double y);

private:
	friend class Mouse;
	//------------------------------ Call Backs ------------------------------------
	// Input
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_enter_callback(GLFWwindow* window, int entered);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void joystick_callback(int joy, int event);
};

#endif