#include "Input.h"

#include "Application.h"
#include "LogFile.h"
#include "utils.h"
#include "RenderWindow.h"
#include "EventManager.h"
#include "KeyEvent.h"

#include <string>

std::map<int, int> Input::Keys;

Input::~Input()
{
	if(Mouse::Instance())
		delete Mouse::Instance();
}

int Input::Init()
{
	{
		// Setup Hash
		Input::Keys[GLFW_KEY_UNKNOWN] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_SPACE] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_APOSTROPHE] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_COMMA] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_MINUS] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_PERIOD] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_SLASH] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_0] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_1] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_2] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_3] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_4] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_5] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_6] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_7] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_8] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_9] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_SEMICOLON] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_EQUAL] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_A] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_B] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_C] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_D] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_E] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_G] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_H] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_I] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_J] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_K] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_L] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_M] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_N] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_O] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_P] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_Q] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_R] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_S] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_T] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_U] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_V] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_W] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_X] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_Y] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_Z] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_LEFT_BRACKET] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_BACKSLASH] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_RIGHT_BRACKET] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_GRAVE_ACCENT] = GLFW_RELEASE;;
		Input::Keys[GLFW_KEY_WORLD_1] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_WORLD_2] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_ENTER] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_TAB] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_BACKSPACE] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_INSERT] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_DELETE] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_RIGHT] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_LEFT] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_DOWN] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_UP] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_PAGE_UP] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_PAGE_DOWN] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_HOME] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_END] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_CAPS_LOCK] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_SCROLL_LOCK] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_NUM_LOCK] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_PRINT_SCREEN] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_PAUSE] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F1] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F2] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F3] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F4] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F5] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F6] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F7] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F8] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F9] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F10] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F11] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F12] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F13] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F14] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F15] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F16] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F17] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F18] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F19] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F20] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F21] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F22] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F23] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F24] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_F25] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_0] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_1] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_2] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_3] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_4] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_5] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_6] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_7] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_8] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_9] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_DECIMAL] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_DIVIDE] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_MULTIPLY] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_SUBTRACT] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_ADD] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_ENTER] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_KP_EQUAL] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_LEFT_SHIFT] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_LEFT_CONTROL] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_LEFT_ALT] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_LEFT_SUPER] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_RIGHT_SHIFT] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_RIGHT_CONTROL] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_RIGHT_ALT] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_RIGHT_SUPER] = GLFW_RELEASE;
		Input::Keys[GLFW_KEY_MENU] = GLFW_RELEASE;
	}	

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
	Keys[key] = action;
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
	if ( button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS )//|| action == GLFW_REPEAT)
			Mouse::Instance()->RMB = true;
		else
			Mouse::Instance()->RMB = false;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS )//|| action == GLFW_REPEAT)
			Mouse::Instance()->LMB = true;
		else
			Mouse::Instance()->LMB = false;
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
