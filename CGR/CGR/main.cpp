#include "src\Application.h"

int main(void)
{
	Application* app = new Application();

	if (app->Init(640, 480, true, "changme", 0, 4, 3, 3, 3))
	{
		// User Adds scenes here
		
		app->Run();
	}

	SAFE_CLOSE(app);
	return 0;
}