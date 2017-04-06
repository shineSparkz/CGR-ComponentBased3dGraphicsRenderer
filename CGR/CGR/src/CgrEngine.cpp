#include "CgrEngine.h"

#include "GameObject.h"
#include "PointLight.h"
#include "SpotLight.h"

GameObject* CgrEngine::CreatePointLight(const Vec3& position, const Vec3& colour, float intensity)
{
	const float ATTEN_CONST = 0.3f;
	const float ATTEN_LIN = 0.0174f;
	const float ATTEN_QUAD = 0.000080f;

	GameObject* pointLight = new GameObject();
	PointLightC* light = pointLight->AddComponent<PointLightC>();

	if (!light->SetLight(position, colour, intensity, ATTEN_CONST, ATTEN_LIN, ATTEN_QUAD))
	{
		SAFE_CLOSE(pointLight);
		return nullptr;
	}

	return pointLight;
}

GameObject* CgrEngine::CreateSpotLight(const Vec3& position, const Vec3& colour, const Vec3& direction, float angle, int switchedOn)
{
	const float ATTEN_CONST = 0.3f;
	const float ATTEN_LIN = 0.0174f;
	const float ATTEN_QUAD = 0.000080f;

	GameObject* spotLight = new GameObject();
	SpotLightC* light = spotLight->AddComponent<SpotLightC>();

	if (!light->SetLight(position, direction, colour, angle, ATTEN_CONST, ATTEN_LIN, ATTEN_QUAD, switchedOn))
	{
		SAFE_CLOSE(spotLight);
		return nullptr;
	}

	return spotLight;
}
