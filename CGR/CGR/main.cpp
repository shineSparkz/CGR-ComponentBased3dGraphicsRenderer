#include "src\Application.h"

#include "src\SponzaScene.h"

int main(void)
{
	
	Application* app = new Application();

	if (app->Init(1280, 720, true, "CGR Render Engine - Alex Spellman", 0, 16, 9, 4, 5))
	{
		// User Adds scenes here
		if (app->AddScene<SponzaScene>(new SponzaScene("sponza")) != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}

		app->ChangeScene("sponza");
		
		app->Run();
	}

	SAFE_CLOSE(app);
	return 0;
}