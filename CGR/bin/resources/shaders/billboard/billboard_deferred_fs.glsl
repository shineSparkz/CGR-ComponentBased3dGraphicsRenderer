#version 450

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
	if (diffuse.a == 0)
	{
		diffuseOut = vec3(0.0);
		discard;
	}
	else
		diffuseOut = diffuse.xyz;
	
	worldPosOut = geom_position; 
    normalOut = normalize(geom_normal); 
	texcoordOut = vec3(geom_texcoord, 0.0); 
}

