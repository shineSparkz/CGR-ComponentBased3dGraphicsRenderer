#version 450

/*
// Deferred
in vec2 geom_texcoord; 
in vec3 geom_normal; 
in vec3 geom_position; 

layout (location = 0) out vec3 worldPosOut; 
layout (location = 1) out vec3 diffuseOut; 
layout (location = 2) out vec3 normalOut; 
layout (location = 3) out vec3 texcoordOut; 

uniform sampler2D u_TextureMap; 

void main()
{
	vec4 diffuse = texture2D(u_TextureMap, geom_texcoord);
	//if (diffuse.a == 0)
	//	discard;	
	diffuseOut = diffuse.xyz;

	worldPosOut = geom_position; 
    normalOut = normalize(geom_normal); 
	texcoordOut = vec3(geom_texcoord, 0.0); 
	
	//diffuseOut = texture2D(u_TextureMap, geom_texcoord).xyz;
}
*/

// TODO : Pass in directional light data from scene, this shader will not support deffered 

// Forward
uniform sampler2D u_TextureMap;

in vec2 geom_texcoord; 
in vec3 geom_normal; 
in vec3 geom_position; 

out vec4 frag_colour;

vec4 GetDirectionalLightColor(vec3 vNormal)
{
	// Hardcode for now
	vec3 dirLightDir = vec3(1,-1,0);
	vec3 dirLightCol = vec3(1.0);
	float dirLightAmbient = 0.1;
	float dirLightDiff = 0.1;

	float fDiffuseIntensity = max(0.0, dot(vNormal, -dirLightDir));
	float fMult = clamp(dirLightAmbient + dirLightDiff, 0.0, 1.0);
	return vec4(dirLightCol * fMult, 1.0);
}

void main()
{	
	vec4 diffuse = texture2D(u_TextureMap, geom_texcoord);     
	if(diffuse.a == 0.0f) 
		discard;
	frag_colour = diffuse * GetDirectionalLightColor(normalize(geom_normal));       
}
