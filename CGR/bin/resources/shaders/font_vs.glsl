#version 330 core

layout (location = 0) in vec4 vertex_position;
out vec2 varying_texcoords;
uniform mat4 proj_xform;

void main()
{
	gl_Position = proj_xform * vec4(vertex_position.xy, 0.0, 1.0);
	varying_texcoords = vertex_position.zw;
}