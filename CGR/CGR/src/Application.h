#ifndef __Application_H__
#define __Application_H__

#include "types.h"
#include "SceneGraph.h"
#include "Singleton.h"
#include "EventHandler.h"
#include <string>

class RenderWindow;
class Renderer;
class ResourceManager;
class Renderer;
class Input;

class Application : public Singleton<Application>, public EventHandler
{
public:
	Application();

	bool Init(int width, int height, bool windowed, const char* title, int vsync, int aspX, int aspY, int major = 3, int minor = 3);
	void Run();
	void Close();

	RenderWindow* GetRenderWindow();

	// Event and Scene Management
	template<typename T> int AddScene(T* state);
	int ChangeScene(const std::string& firstState);

private:
	void HandleEvent(Event* ev) override;
	static void glfw_error_callback(int error, const char* description);

private:
	SceneGraph* m_SceneGraph;
	RenderWindow* m_RenderWindow;
	ResourceManager* m_ResourceManager;
	Renderer* m_Renderer;
	Input* m_Input;

	int m_ShouldClose;
	int m_PendingSceneChange;
	int m_PendingSceneHash;
};

inline RenderWindow* Application::GetRenderWindow()
{
	return m_RenderWindow;
}

template<typename T>
int Application::AddScene(T* state)
{
	if (!m_SceneGraph || !m_Renderer)
	{
		return GE_MAJOR_ERROR;
	}

	return m_SceneGraph->AddScene(state, m_Renderer);
}

#endif
