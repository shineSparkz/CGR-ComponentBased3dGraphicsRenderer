#include "src\Application.h"

int main(void)
{
	Application* app = new Application();

	if (app->Init(1280, 720, true, "CGR Render Engine - Alex Spellman", 0, 16, 9, 3, 3))
	{
		// User Adds scenes here
		
		app->Run();
	}

	SAFE_CLOSE(app);
	return 0;
}