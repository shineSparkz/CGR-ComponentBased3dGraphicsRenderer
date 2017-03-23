#ifndef __ISTATE_H__
#define __ISTATE_H__

#include "types.h"
#include <string>

class BaseCamera;
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

	// Loading resources
	virtual int  OnSceneCreate() = 0;
	
	// Allocate scene specific game objects
	virtual int  OnSceneLoad() = 0;

	// Delete scene specific objects
	virtual void OnSceneExit() = 0;

	// Loop
	virtual void Update(float dt) = 0;
	virtual void Render(Renderer* renderer) = 0;

	BaseCamera* GetActiveCamera();
	const std::string& GetName() const;

protected:
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