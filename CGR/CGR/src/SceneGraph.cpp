#include "SceneGraph.h"

#include "LogFile.h"
#include "IScene.h"
#include "EventManager.h"
#include "utils.h"

SceneGraph::SceneGraph() :
	m_Scenes(),
	m_ActiveScene(-1)
{
}

SceneGraph::~SceneGraph()
{
}

bool SceneGraph::IsEmpty() const
{
	return m_Scenes.empty();
}

IScene* SceneGraph::GetActiveScene()
{
	return m_Scenes[m_ActiveScene];
}

void SceneGraph::UpdateActiveScene(float dt)
{
	m_Scenes[m_ActiveScene]->Update(dt);
}

void SceneGraph::RenderActiveScene(Renderer* renderer)
{
	m_Scenes[m_ActiveScene]->Render(renderer);
}

void SceneGraph::ChangeScene(int newState)
{
	if (m_Scenes.find(newState) == m_Scenes.end())
	{
		std::string er = "StateStack could not set state because it doesn't exist.";
		WRITE_LOG(er, "error");
		return;
	}

	// For first time
	if (m_ActiveScene == -1)
	{
		m_ActiveScene = newState;
	}
	else
	{
		m_Scenes[m_ActiveScene]->OnSceneExit();
		m_ActiveScene = newState;
	}

	if (m_Scenes[m_ActiveScene]->OnSceneLoad() != GE_OK)
	{
		// Error event
		std::string err = "Error: scene : " + m_Scenes[m_ActiveScene]->GetName() + " load failed";
		//EventManager::Instance()->SendEvent(events::EventID::ErrorEvent, &err);
	}
}

void SceneGraph::ChangeSceneByName(const std::string& nextStateStr)
{
	int newState =  util::str_hash(nextStateStr);
	this->ChangeScene(newState);
}

int SceneGraph::GetActiveSceneHash() const
{
	return m_ActiveScene;
}

const std::string& SceneGraph::GetActiveSceneName() 
{
	return m_Scenes[m_ActiveScene]->GetName();
}

void SceneGraph::Close()
{
	if (!m_Scenes.empty())
	{
		std::map<int, IScene*>::iterator it;
		
		// Close active one
		if(m_Scenes.find(m_ActiveScene) != m_Scenes.end())
			m_Scenes[m_ActiveScene]->OnSceneExit();

		for (it = m_Scenes.begin(); it != m_Scenes.end(); ++it)
		{
			SAFE_DELETE(it->second);
		}

		m_Scenes.clear();
	}
}

int SceneGraph::HashHelper(const std::string& s)
{
	return util::str_hash(s);
}
