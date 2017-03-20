#include "Application.h"

#include "LogFile.h"
#include "EventManager.h"
#include "Time.h"
#include "RenderWindow.h"
#include "Renderer.h"
#include "Input.h"
#include "math_utils.h"
#include "KeyEvent.h"
#include "Screen.h"

Application::Application() :
	m_SceneGraph(nullptr),
	m_RenderWindow(nullptr),
	m_ResourceManager(nullptr),
	m_Renderer(nullptr),
	m_Input(nullptr),
	m_ShouldClose(GE_FALSE)
{
}

bool Application::Init(int width, int height, bool windowed, const char* title, int vsync, int aspX, int aspY, int major, int minor)
{
	// ** In theory... This funciton would be completly re-written for each different platform, it sets up back end
	//				   rendering, and hardware specific input etc.

	// Check memory leaks
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Create Logger first
	new DebugLogFile();
	DebugLogFile::Instance()->CreateLogFile("../resources/log/", "logfile.html");

	// Init Hardware, window and input backend
	if (!glfwInit())
	{
		WRITE_LOG("Error: Failed to Init GLFW API.", "error");
		return false;
	}

	// Log API version
	std::string glfwVersion = glfwGetVersionString();
	WRITE_LOG("Using GLFW version: " + glfwVersion, "none");

	// Open a window
	if (!m_RenderWindow)
	{
		m_RenderWindow = new RenderWindow();
	}

	new Screen();

	if (!m_RenderWindow->Open(width,height,windowed,title,vsync,aspX,aspY,major,minor))
	{
		WRITE_LOG("Error opening render window", "error");
		return false;
	}

	// Now init glew for open gl calls, becuase we have a context
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		WRITE_LOG("GLEW INIT failed.", "error");
		return false;
	}

	const char* version = (const char*)glewGetString(GLEW_VERSION);
	std::string glewVersion = "GLEW_VERSION : " + (std::string)version;
	WRITE_LOG(glewVersion, "none");

	// TODO : Create a Screen Singleton class, that can talk to the render window

	// Important to set this initially

	//glViewport(0, 0, s_FrameBuffWidth, s_FrameBuffHeight);
	glfwSwapInterval(Maths::Clamp(vsync, 0, 1));

	// Input system
	if (!m_Input)
	{
		m_Input = new Input();
	}

	if (m_Input->Init() != GE_OK)
	{
		WRITE_LOG("Error: Input init failed", "error");
		return GE_FATAL_ERROR;
	}

	// Set GL STATES should be done by user, or default values set here

	// Event System (Singleton)
	EventManager* em = new EventManager();

	// App wants to know about all engine specific events (user can make their own, or listen for these)
	for (dword i = 0; i < NUM_ENGINE_EVENTS; ++i)
	{
		if (!em->RegisterEvent(i))
		{
			return false;
		}

		if (!em->AttachEvent(i, *this))
		{
			return false;
		}
	}
	
	// TODO : Load default resources once I have created this system

	/*
	if (!m_ResourceManager)
	{
		m_ResourceManager = new ResourceManager();

		if (m_ResourceManager->LoadAllResources() != GE_OK)
		{
			return GE_FATAL_ERROR;
		}
	}
	*/

	// Create Renderer 
	if (!m_Renderer)
	{
		m_Renderer = new Renderer();
	}

	if (!m_Renderer->Init())
	{
		return false;
	}

	return true;
}

int Application::ChangeScene(const std::string& firstState)
{
	if (m_SceneGraph && !m_SceneGraph->IsEmpty())
	{
		m_SceneGraph->ChangeSceneByName(firstState);
		return GE_OK;
	}

	WRITE_LOG("Error: Can't set all states when state manager is null.", "error");
	return GE_MAJOR_ERROR;
}

void Application::Close()
{
	// TODO : Detach all events

	SAFE_DELETE(m_Input);

	//SAFE_CLOSE(m_ResourceManager);
	SAFE_CLOSE(m_Renderer);
	SAFE_CLOSE(m_SceneGraph);
	SAFE_CLOSE(m_RenderWindow);

	// Assumes all events have been detached by now

	glfwTerminate();
	
	delete Screen::Instance();
	delete EventManager::Instance();
	delete DebugLogFile::Instance();
}

void Application::Run()
{
	// TODO : Put this check back later

	// Check that state manager has been set first, need at least one state
	//if (!m_SceneGraph || (m_SceneGraph && m_SceneGraph->IsEmpty()))
	//{
	//	WRITE_LOG("Error: The state manager in applicaiton has not been init, need to add states and call 'InformAllStatesSet'", "error");
	//	return;
	//}

	const float FPS = 60.0f;
	const float DELTA_TICK = 1.0f / FPS;
	const float MAX_FRAME_SKIP = 10;
	float time_now = 0;
	int frame_count = 0;
	float next_tick = 0;

	Timer timer;

	// Update loop
	while (m_RenderWindow->IsOpen() && m_ShouldClose != GE_TRUE)
	{
		timer.Update();

		frame_count = 0;
		time_now = timer.Total();

		Time::elapsedTime = time_now;
		Time::deltaTime = timer.Delta();

		while (time_now > next_tick && frame_count < MAX_FRAME_SKIP)
		{
			//m_SceneGraph->UpdateActiveScene(Time::deltaTime);
			next_tick += DELTA_TICK;
			frame_count++;
		}

		//m_SceneGraph->RenderActiveScene(m_Renderer);
		m_Renderer->Render();
		m_RenderWindow->SwapBuffers();
		glfwPollEvents();
	}

	glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
}

void Application::glfw_error_callback(int error, const char* description)
{
	std::string s = description;
	WRITE_LOG(s, "error");
}

void Application::HandleEvent(Event* e)
{
	switch (e->GetID())
	{
	case KEY_EVENT:
	{
		KeyEvent* ke = (KeyEvent*)e->GetData();

		if (ke)
		{
			if (ke->key == GLFW_KEY_ESCAPE && ke->action == GLFW_RELEASE)
			{
				this->m_ShouldClose = GE_TRUE;
			}
			else if (ke->key == GLFW_KEY_F1 && ke->action == GLFW_RELEASE)
			{
				//this->ChangeScene("shadow");
			}
			else if (ke->key == GLFW_KEY_F2 && ke->action == GLFW_RELEASE)
			{
				//this->ChangeScene("normalmap");
			}
		}
		// else pass this event on to current state
		break;
	}
	case WINDOW_FOCUS_EVENT:
		break;
	case SHUTDOWN_EVENT:
		m_ShouldClose = GE_TRUE;
		break;
	}
}