#include "src\Application.h"

#include "src\SponzaScene.h"
#include "src\OrthoScene.h"
#include "src\IndoorLevelScene.h"
#include "src\OutdoorScene.h"

/*
List
- Reimplement normal maps
- Reimplement shadows Directional, Have shadows for multiple lights
- Wavy grass
- Change shaders at runtime, reload shaders runtime
- Materials
- Instancing/Batching
- Frustum
- Better water implementation

- Make all scenes nice 
- Make a use case for a new user, creating a scene, game objects, shader and using engine functionality or write docs
- Tidy Camera

Extra
- picking
- fog
- AA

Done
- Tidy up terrain shader
- Switch scenes

*/


int main(void)
{
	
	Application* app = new Application();

	if (app->Init(1280, 720, true, "CGR Render Engine - Alex Spellman", 0, 16, 9, 4, 5))
	{
		// User Adds scenes here
		if (app->AddScene<IndoorLevelScene>(new IndoorLevelScene("indoor")) != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}

		if (app->AddScene<OutDoorScene>(new OutDoorScene("outdoor")) != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}

		if (app->AddScene<SponzaScene>(new SponzaScene("sponza")) != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}

		if (app->AddScene<OrthoScene>(new OrthoScene("ortho")) != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}


		if (app->ChangeScene("indoor") != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}
		
		app->Run();
	}

	SAFE_CLOSE(app);
	return 0;
}