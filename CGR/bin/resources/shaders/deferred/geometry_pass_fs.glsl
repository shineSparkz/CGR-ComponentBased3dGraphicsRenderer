#version 450

in vec2 varying_texcoord; 
in vec3 varying_normal; 
in vec3 varying_position; 

layout (location = 0) out vec3 worldPosOut; 
layout (location = 1) out vec3 diffuseOut; 
layout (location = 2) out vec3 normalOut; 
layout (location = 3) out vec3 texcoordOut; 

uniform sampler2D u_ColourMap; 

void main() 
{ 
    worldPosOut = varying_position; 
    diffuseOut = texture(u_ColourMap, varying_texcoord).xyz;
    normalOut = normalize(varying_normal); 
	texcoordOut = vec3(varying_texcoord, 0.0); 
}