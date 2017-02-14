#include "Input.h"

#include "Application.h"
#include "LogFile.h"
#include "utils.h"
#include "RenderWindow.h"
#include "EventManager.h"
#include "KeyEvent.h"

#include <string>

Input::~Input()
{
	if(Mouse::Instance())
		delete Mouse::Instance();
}

int Input::Init()
{
	RenderWindow* win = Application::Instance()->GetRenderWindow();
	new Mouse(this);

	if (!win)
	{
		WRITE_LOG("Error: can't init input without a window", "error");
		return GE_MAJOR_ERROR;
	}

	// Set Input callbacks
	glfwSetKeyCallback(win->m_Window, Input::key_callback);
	glfwSetCursorPosCallback(win->m_Window, Input::mouse_position_callback);
	glfwSetCursorEnterCallback(win->m_Window, Input::mouse_enter_callback);
	glfwSetMouseButtonCallback(win->m_Window, Input::mouse_button_callback);
	glfwSetScrollCallback(win->m_Window, Input::mouse_scroll_callback);
	glfwSetJoystickCallback(Input::joystick_callback);

	// TODO : Controller support
	int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
	if ( present )
	{
		int count;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
		int countb;
		const unsigned char* axes2 = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &countb);
		const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);
	}

	return GE_OK;
}

void Input::SetMousePosition(double x, double y)
{
	if (Application::Instance()->GetRenderWindow())
	{
		glfwSetCursorPos(Application::Instance()->GetRenderWindow()->m_Window, x, y);
	}
}

void Input::GetMousePosition(double& x, double& y)
{
	if (Application::Instance()->GetRenderWindow())
	{
		glfwGetCursorPos(Application::Instance()->GetRenderWindow()->m_Window, &x, &y);
	}
}

//--- Input Callbacks------------------------------
void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	KeyEvent kev{ key, action };
	EventManager::Instance()->SendEvent(KEY_EVENT, &kev);
}

void Input::mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Mouse* m = Mouse::Instance();
	if (m)
	{
		m->m_Xpos = xpos;
		m->m_Ypos = ypos;
	}
}

void Input::mouse_enter_callback(GLFWwindow* window, int entered)
{
	if ( entered )
	{
		// The cursor entered the client area of the window
	}
	else
	{
		// The cursor left the client area of the window
	}
}

void Input::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if ( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS )
	{
		//popup_menu();
		WRITE_LOG("mouse right pressed", "normal");
	}
}

void Input::mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
}

void Input::joystick_callback(int joy, int event)
{
	std::string s = "joystick callback: joy:" + util::to_str(joy) +
		" event:" + util::to_str(event);
	WRITE_LOG(s, "normal");

	if ( event == GLFW_CONNECTED )
	{
		// The joystick was connected
	}
	else if ( event == GLFW_DISCONNECTED )
	{
		// The joystick was disconnected
	}
}


// ---- Mouse ----
Mouse::Mouse(Input* input) :
	m_InputManager(input),
	m_Xpos(0.0),
	m_Ypos(0.0)
{

}

double Mouse::PosX() const
{
	return m_Xpos;
}

double Mouse::PosY() const
{
	return m_Ypos;
}

void Mouse::SetMousePosition(double x, double y)
{
	m_Xpos = x;
	m_Ypos = y;
	m_InputManager->SetMousePosition(m_Xpos, m_Ypos);
}

void Mouse::GetMousePosition(double& x, double& y)
{
	m_InputManager->GetMousePosition(x, y);
	m_Xpos = x;
	m_Ypos = y;
}
