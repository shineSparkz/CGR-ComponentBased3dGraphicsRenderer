#ifndef __SCENE_GRAPH_H__
#define __SCENE_GRAPH_H__

#include <map>
#include <string>

class IScene;
class Renderer;
class ResourceManager;
class Renderer;

class SceneGraph
{
public:
	SceneGraph();
	~SceneGraph();

	// Returns the hashed string, will return GE_FATAL_ERROR if problem
	template<typename T>
	int AddScene(T* state, Renderer* renderer);

	bool IsEmpty() const;

	void UpdateActiveScene(float dt);
	void RenderActiveScene(int renderUI);

	void Close();
	void ChangeScene(int nextState, ResourceManager* resManager);
	void ChangeSceneByName(const std::string& nextState, ResourceManager* resManager);

	IScene* GetActiveScene();
	int GetActiveSceneHash() const;
	const std::string& GetActiveSceneName();
	int HashHelper(const std::string& s);

private:
	std::map<int, IScene*>	m_Scenes;
	int						m_ActiveScene;
};

template<typename T>
int SceneGraph::AddScene(T* state, Renderer* renderer)
{
	int hash = this->HashHelper(state->GetName());
	if (m_Scenes.find(hash) == m_Scenes.end())
	{
		m_Scenes[hash] = state;
		return m_Scenes[hash]->OnSceneCreate(renderer);
	}

	return GE_FATAL_ERROR;
} 

#endif