#ifndef __ISTATE_H__
#define __ISTATE_H__

#include "types.h"
#include <string>

class BaseCamera;
class Renderer;
class ResourceManager;
class Renderer;

class IScene
{
public:
	IScene(const std::string& name) :
		m_Camera(nullptr),
		m_Name(name)
	{
	}
	
	virtual ~IScene() {}

	// Use this to Load one off resources
	virtual int  OnSceneCreate(Renderer* renderer);
	
	// Use this to allocate scene specific game objects and components - Dont forget to set scene data in the renderer
	virtual int  OnSceneLoad(ResourceManager* resManager) = 0;

	// Use this to Delete scene specific objects
	virtual void OnSceneExit() = 0;

	// Add update logic here
	virtual void Update(float dt) = 0;
	
	// Ask renderer class to render scene here
	virtual void Render() = 0;

	BaseCamera* GetActiveCamera();
	const std::string& GetName() const;

protected:
	Renderer* m_Renderer;	// <-- Weak Ptr
	BaseCamera* m_Camera;
	std::string m_Name;
};

INLINE BaseCamera* IScene::GetActiveCamera()
{
	return m_Camera;
}

INLINE const std::string& IScene::GetName() const
{
	return m_Name;
}

#endif