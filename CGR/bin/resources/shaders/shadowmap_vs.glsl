#version 330

layout(location=0) in vec3 vertex_position;
layout(location=1) in vec3 vertex_normal;
layout(location=2) in vec2 vertex_texcoord;

uniform mat4 u_wvp_xform;

out vec2 varying_texcoord;

void main()
{
	gl_Position = u_wvp_xform * vec4(vertex_position, 1.0);
	varying_texcoord = vertex_texcoord;
}