#include "src\Application.h"

#include "src\SponzaScene.h"
#include "src\OrthoScene.h"
#include "src\IndoorLevelScene.h"

/*
List

- Fix index issue with uniform buffer
- Make it so can have array in block
- Add spots and points to block
- Update all or any shaders that canm use the block

- Reimplement shadows
- Have shadows for multiple lights
- reimplement normal maps
- Change all default meshes to use tangents

- Tidy all shaders to share blocks

- Use sub routines to toggle lighting

- Wavy grass
- 

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