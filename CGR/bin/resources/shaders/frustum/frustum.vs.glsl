#version 450

layout (location = 0)   in  vec3 vertex_position;

uniform mat4 u_wvp_xform;

void main()
{
	gl_Position = u_wvp_xform * vec4(vertex_position, 1.0);
}