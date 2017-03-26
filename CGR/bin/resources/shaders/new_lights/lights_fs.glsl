#version 450

/*
struct DirectionalLight
{
    vec3 direction; 
    vec3 intensity;
	float ambient_intensity;
};

layout (std140) uniform DirectionalLights
{
    #define DirectionalLightsMax 5
    uint                count;
    DirectionalLight    lights[DirectionalLightsMax];  
} directionalLights;
*/

#include_part

layout(std140) uniform DirectionalLight
{
	vec3 direction;
	vec3 intensity;
};

vec4 getDirectionalLightColour(in vec3 n);

#definition_part

vec4 getDirectionalLightColour(in vec3 n)
{
	//DirectionalLight dirLight = directionalLight; // directionalLights.lights[index];
	float diffuse_intensity = max(0.0, dot(n, -direction));
	float fMult = clamp(0.25 + diffuse_intensity, 0.0, 1.0);
	return vec4(intensity * fMult, 1.0);
}

/*
vec4 directionalLightContributions (const in vec3 normal)
{
    vec4 lighting = vec4 (0.0);

    for (uint i = 0; i < directionalLights.count; ++i)
    {
        lighting += getDirectionalLightColour (i, normal);
    }

    return lighting;
}
*/