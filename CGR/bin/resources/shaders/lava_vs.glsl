#version 330

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_texcoord;

uniform mat4 u_WvpXform;
out vec2 varying_texcoord;

void main()
{
	varying_texcoord = vertex_texcoord;	
	gl_Position = u_WvpXform * vec4(vertex_position, 1.0);
}