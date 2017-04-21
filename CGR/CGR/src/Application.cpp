#include "Application.h"

#include "utils.h"
#include "LogFile.h"
#include "EventManager.h"
#include "Time.h"
#include "RenderWindow.h"
#include "Renderer.h"
#include "Input.h"
#include "math_utils.h"
#include "KeyEvent.h"
#include "Screen.h"
#include "ResId.h"
#include "Mesh.h"

Application::Application() :
	m_SceneGraph(nullptr),
	m_RenderWindow(nullptr),
	m_ResourceManager(nullptr),
	m_Renderer(nullptr),
	m_Input(nullptr),
	m_ShouldClose(GE_FALSE),
	m_PendingSceneChange(GE_FALSE),
	m_ShouldRendedInfoStrings(GE_FALSE),
	m_ShouldRenderSceneUI(GE_FALSE)
{
	// Create Logger first
	if (!DebugLogFile::Instance())
	{
		new DebugLogFile();
		DebugLogFile::Instance()->CreateLogFile("../resources/log/", "logfile.html");
	}

#ifdef  _DEBUG
	m_ShouldRendedInfoStrings = GE_TRUE;
#endif //  _DEBUG

}

bool Application::Init(int width, int height, bool windowed, const char* title, int vsync, int aspX, int aspY, int major, int minor)
{
	// ** In theory... This function would be completly re-written for each different platform, it sets up back end
	//				   rendering, and hardware specific input etc.

	// Check memory leaks
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	if (!m_SceneGraph)
		m_SceneGraph = new SceneGraph();

	// Init Hardware, window and input backend
	if (!glfwInit())
	{
		WRITE_LOG("Error: Failed to Init GLFW API.", "error");
		return false;
	}

	// Log API version
	std::string glfwVersion = glfwGetVersionString();
	WRITE_LOG("Using GLFW version: " + glfwVersion, "none");

	glfwSetErrorCallback(Application::glfw_error_callback);

	// Open a window
	if (!m_RenderWindow)
	{
		m_RenderWindow = new RenderWindow();
	}

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
	
	// Create Renderer 
	if (!m_Renderer)
	{
		m_Renderer = new Renderer();
	}

	if (!m_Renderer->Init())
	{
		return false;
	}

	m_Renderer->SetDisplayInfo(m_ShouldRendedInfoStrings == GE_TRUE);

	return true;
}

int Application::ChangeScene(const std::string& state)
{
	m_PendingSceneChange = GE_TRUE;
	m_PendingSceneHash = m_SceneGraph->HashHelper(state);
	return GE_OK;
}

void Application::Close()
{
	// TODO : Detach all events

	SAFE_DELETE(m_Input);
	SAFE_CLOSE(m_Renderer);
	SAFE_CLOSE(m_SceneGraph);
	SAFE_CLOSE(m_RenderWindow);

	// Assumes all events have been detached by now

	glfwTerminate();
	
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
		// See if the scene needs changing, make sure everything has been set first
		if (m_PendingSceneChange == GE_TRUE)
		{
			m_PendingSceneChange = GE_FALSE;

			if (m_SceneGraph && !m_SceneGraph->IsEmpty())
			{
				m_SceneGraph->ChangeScene(m_PendingSceneHash, m_Renderer->GetResourceManager());
			}
		}

		timer.Update();

		frame_count = 0;
		time_now = timer.Total();

		Time::elapsedTime = time_now;
		Time::deltaTime = timer.Delta();

		while (time_now > next_tick && frame_count < MAX_FRAME_SKIP)
		{
			m_SceneGraph->UpdateActiveScene(Time::deltaTime);
			next_tick += Time::deltaTime;
			frame_count++;
		}

		m_SceneGraph->RenderActiveScene(m_ShouldRenderSceneUI);
		
		if (m_ShouldRendedInfoStrings)
			renderInfo();

		m_RenderWindow->SwapBuffers();
		glfwPollEvents();
	}

	glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
}

void Application::renderInfo()
{
	int numItems = 2;		// <-- Hacky, it's from renderer
	const float divider = 32.0f;
	const float top = static_cast<float>(Screen::FrameBufferHeight());

	if(!m_ShouldRenderSceneUI)
		m_Renderer->RenderText(FONT_COURIER, "Press [Tab] to toggle scene UI layer", 8, 32);
	
	m_Renderer->RenderText(FONT_COURIER, m_Renderer->GetHardwareStr(),		8, top - (++numItems * divider), FontAlign::Left, Colour::Blue());
	m_Renderer->RenderText(FONT_COURIER, m_Renderer->GetShadingModeStr(),	8, top - (++numItems * divider), FontAlign::Left, Colour::Blue());
	m_Renderer->RenderText(FONT_COURIER, "Displaying Normals: " + util::bool_to_str(m_Renderer->IsDisplayingNormals()), 8, top - (++numItems * divider), FontAlign::Left, Colour::Blue());
	m_Renderer->RenderText(FONT_COURIER, "Num Verts: " + util::to_str(Mesh::NumVerts) + ", Num Meshes: " + util::to_str(Mesh::NumMeshes), 8, top - (++numItems * divider), FontAlign::Left, Colour::Green());
	m_Renderer->RenderText(FONT_COURIER, m_SceneGraph->GetActiveSceneName() + " scene example", 8, top - (++numItems * divider), FontAlign::Left, Colour::Red());
}

void Application::ShouldRenderInfoStrings(bool should)
{
	should ? m_ShouldRendedInfoStrings = GE_TRUE : m_ShouldRendedInfoStrings = GE_FALSE;
}

bool Application::IsRenderingInfoStrings() const
{
	if (m_ShouldRendedInfoStrings == GE_TRUE)
		return true;
	return false;
}

void Application::glfw_error_callback(int error, const char* description)
{
	std::stringstream ss;
	ss << "GLFW error call back: Errorcode[";
	ss << error;
	ss << "] , Description: ";
	ss << description;

	WRITE_LOG(ss.str(), "error");
}

void Application::HandleEvent(Event* e)
{
	switch (e->GetID())
	{
	case EVENT_KEY:
	{
		KeyEvent* ke = (KeyEvent*)e->GetData();

		if (ke)
		{
			if (ke->key == GLFW_KEY_ESCAPE && ke->action == GLFW_RELEASE)
			{
				this->m_ShouldClose = GE_TRUE;
			}

			// Indoor Scene
			else if (ke->key == GLFW_KEY_F1 && ke->action == GLFW_RELEASE)
			{
				this->ChangeScene("indoor");
			}
			// Outdoor Scene
			else if (ke->key == GLFW_KEY_F2 && ke->action == GLFW_RELEASE)
			{
				this->ChangeScene("outdoor");
			}
			// Sponza Scene
			else if (ke->key == GLFW_KEY_F3 && ke->action == GLFW_RELEASE)
			{
				this->ChangeScene("sponza");
			}
			// Ortho Scene
			else if (ke->key == GLFW_KEY_F4 && ke->action == GLFW_RELEASE)
			{
				this->ChangeScene("ortho");
			}
			// Toggle Culling
			else if (ke->key == GLFW_KEY_F9 && ke->action == GLFW_RELEASE)
			{
				this->m_Renderer->ToggleFrustumCulling();
			}
			// Toggle Frame Quuery
			else if (ke->key == GLFW_KEY_F10 && ke->action == GLFW_RELEASE)
			{
				this->m_Renderer->ToggleFrameQueeryMode();
			}
			// Toggle Info strings
			else if (ke->key == GLFW_KEY_F11 && ke->action == GLFW_RELEASE)
			{
				this->ShouldRenderInfoStrings(!this->IsRenderingInfoStrings());
				m_Renderer->SetDisplayInfo(this->IsRenderingInfoStrings());
			}
			else if (ke->key == GLFW_KEY_TAB && ke->action == GLFW_RELEASE)
			{
				if (m_ShouldRenderSceneUI)
					m_ShouldRenderSceneUI = GE_FALSE;
				else
					m_ShouldRenderSceneUI = GE_TRUE;
			}
		}
		// else pass this event on to current state
		break;
	}
	case EVENT_WINDOW_FOCUS:
		break;
	case EVENT_SHUTDOWN:
		m_ShouldClose = GE_TRUE;
		break;
	}
}