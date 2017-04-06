#include "src\Application.h"

#include "src\SponzaScene.h"
#include "src\OrthoScene.h"
#include "src\IndoorLevelScene.h"

/*
List

- Tidy up terrain shader
- Refactor Terrain stuff into mesh renderer (hopefully)
- Reimplement normal maps
- Reimplement shadows Directional, Have shadows for multiple lights
- Wavy grass
- Change shaders at runtime, reload shaders runtime
- Materials
- Instancing/Batching
- Frustum
- Better water implementation

- Make all scenes nice and switch between them
- Make a use case for a new user, creating a scene, game objects, shader and using engine functionality or write docs

Extra
- picking
- fog
- AA

*/


int main(void)
{
	
	Application* app = new Application();

	if (app->Init(1280, 720, true, "CGR Render Engine - Alex Spellman", 0, 16, 9, 4, 5))
	{
		// User Adds scenes here
		if (app->AddScene<IndoorLevelScene>(new IndoorLevelScene("ortho")) != GE_OK)
		//if(app->AddScene<SponzaScene>(new SponzaScene("ortho")) != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}

		if (app->ChangeScene("ortho") != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}
		
		app->Run();
	}

	SAFE_CLOSE(app);
	return 0;
}