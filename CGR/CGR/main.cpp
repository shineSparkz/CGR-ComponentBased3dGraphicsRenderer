#include "src\Application.h"

#include "src\SponzaScene.h"
#include "src\OrthoScene.h"
#include "src\IndoorLevelScene.h"
#include "src\OutdoorScene.h"

/*
List
- Wavy grass
- Reimplement shadows Directional, Have shadows for multiple lights
- Instancing/Batching
- Frustum
- Change shaders at runtime, reload shaders runtime
- Better water implementation
- Materials
- Make a use case for a new user, creating a scene, game objects, shader and using engine functionality or write docs

Extra
- picking
- fog
- AA

Done
- Tidy up terrain shader
- Switch scenes
- Normal display tool
- Specular
- Reimplement normal maps

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


		if (app->ChangeScene("outdoor") != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}
		
		app->Run();
	}

	SAFE_CLOSE(app);
	return 0;
}