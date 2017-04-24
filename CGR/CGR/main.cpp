#include "src\Application.h"

#include "src\SponzaScene.h"
#include "src\OrthoScene.h"
#include "src\IndoorLevelScene.h"
#include "src\OutdoorScene.h"
#include "src\SpaceScene.h"
#include "src\VivaScene.h"
#include "src\TextFile.h"
#include "src\utils.h"

/*
List
- Make a use case for a new user, creating a scene, game objects, shader and using engine functionality or write docs

Done
- Fix deferred
- Make scenes nice and add more meshes etc
- Fix normals
- Tidy all code
- Tidy up terrain shader
- Switch scenes
- Normal display tool
- Specular
- Reimplement normal maps
- Frustum
- Bez Terrains
- Materials
- Reimplement shadows Directional
- Wavy grass
- Change shaders at runtime, reload shaders runtime

Cuts
- Instancing/Batching
- AA
- Particles
- picking
- fog
- Better water implementation
*/


void resolveIniFile(int& windowed, int& resolution_width, int& resolution_height, std::string& scene_load, int& vsync, int& major, int& minor);

int main(void)
{
	int windowed = 1;
	int resolution_width = 1280;
	int resolution_height = 720;
	int vsync = 0;
	int gl_major = 4;
	int gl_minor = 5;
	std::string scene_load = "outdoor";
	resolveIniFile(windowed, resolution_width, resolution_height, scene_load, vsync, gl_major, gl_minor);
	
	Application* app = new Application();

	if (app->Init(resolution_width, resolution_height, windowed == GE_TRUE, "CGR Render Engine - Alex Spellman", vsync, 16, 9, gl_major, gl_minor))
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

		if (app->AddScene<SpaceScene>(new SpaceScene("space")) != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}

		if (app->AddScene<VivaScene>(new VivaScene("viva")) != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}

		if (app->ChangeScene(scene_load) != GE_OK)
		{
			SAFE_CLOSE(app);
			return -1;
		}
		
		app->Run();
	}

	SAFE_CLOSE(app);
	return 0;
}


void resolveIniFile(int& windowed, int& resolution_width, int& resolution_height, std::string& scene_load, int& vsync, int& major, int& minor)
{
	TextFile cgr_ini;
	if (!cgr_ini.LoadFileAsLinesToBuffer("../cgr.ini"))
	{
		std::cout << "Error with missing ini file";
	}
	else
	{
		std::vector<std::string> buff = cgr_ini.GetBuffer();
		if (buff.size() == 7)
		{
			auto s = util::split_str(buff[0], ':');
			if (s.size() > 1)
			{
				windowed = util::str_to_int(s[1].c_str());
			}

			s = util::split_str(buff[1], ':');
			if (s.size() > 1)
			{
				resolution_width = util::str_to_int(s[1].c_str());
			}

			s = util::split_str(buff[2], ':');
			if (s.size() > 1)
			{
				resolution_height = util::str_to_int(s[1].c_str());
			}

			s = util::split_str(buff[3], ':');
			if (s.size() > 1)
			{
				scene_load = s[1];
			}

			s = util::split_str(buff[4], ':');
			if (s.size() > 1)
			{
				vsync = util::str_to_int(s[1].c_str());
			}

			s = util::split_str(buff[5], ':');
			if (s.size() > 1)
			{
				major = util::str_to_int(s[1].c_str());
			}

			s = util::split_str(buff[6], ':');
			if (s.size() > 1)
			{
				minor = util::str_to_int(s[1].c_str());
			}
		}
	}
}