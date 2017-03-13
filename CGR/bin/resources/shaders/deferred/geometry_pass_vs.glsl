#version 330

layout (location = 0) in vec3 vertex_position; 
layout (location = 1) in vec3 vertex_normal; 
layout (location = 2) in vec2 vertex_texcoord; 

uniform mat4 u_WVP;
uniform mat4 u_World;

out vec2 varying_texcoord; 
out vec3 varying_normal; 
out vec3 varying_position; 

void main()
{ 
    gl_Position = u_WVP * vec4(vertex_position, 1.0);
    varying_texcoord = vertex_texcoord; 
    varying_normal = (u_World * vec4(vertex_normal, 0.0)).xyz;
    varying_position = (u_World * vec4(vertex_position, 1.0)).xyz;
}