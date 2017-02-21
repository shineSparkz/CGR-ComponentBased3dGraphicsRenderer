#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "gl_headers.h"
#include "types.h"
#include "Lights.h"
#include "Colour.h"
#include "Vertex.h"
#include "FontAlign.h"
#include <vector>

class Mesh;
class LightingTechnique;
class BasicDiffuseTechnique;
class Camera;
class Font;
class Texture;
class FontTechnique;
class SkyboxTechnique;

class Renderer
{
public:
	Renderer();

	bool Init();
	void Render();
	void Close();
	void ReloadShaders();

	void RenderText(const std::string& txt, float x, float y, FontAlign fa = FontAlign::Left, const Colour& col = Colour::White() );
	Texture* GetTexture(size_t index);

private:
	// ---- Keeping ----
	FontTechnique* m_FontTechnique{ nullptr };

	// ---- Removing ----
	std::vector<Mesh*> m_Meshes;
	std::vector<Texture*> m_Textures;
	Mesh* m_SkyboxMesh;

	Font* m_Font{ nullptr };
	//GLuint m_FontShaderProg;
	Camera* m_Camera{ nullptr };

	LightingTechnique* m_pEffect{ nullptr };
	BasicDiffuseTechnique* m_DiffuseMat{ nullptr };
	SkyboxTechnique* m_SkyBoxMat{ nullptr };
	
	// These would be in the scene, maybe even component based
	DirectionalLight m_directionalLight;
	SpotLight m_SpotLights[2];
	PointLight m_PointLights[2];
};

#endif