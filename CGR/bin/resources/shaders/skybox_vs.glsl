#version 330 core

layout(location=0)
in vec3 vertex_position;

uniform mat4 wvp_xform;

out vec3 varying_texcoord;

void main()
{
	vec4 wvp = wvp_xform * vec4(vertex_position, 1.0);
	gl_Position = wvp.xyww;
	varying_texcoord = vertex_position;
}