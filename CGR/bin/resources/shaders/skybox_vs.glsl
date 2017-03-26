#version 450 core

layout (binding = 1, std140) uniform scene
{ 
	mat4 proj_xform;
	mat4 view_xform;
	vec3 camera_position;
	vec3 ambient_light;
};

uniform mat4 world_xform;

layout(location=0) in vec3 vertex_position;

out vec3 varying_texcoord;

void main()
{
	vec4 wvp = proj_xform * view_xform * world_xform * vec4(vertex_position, 1.0);
	gl_Position = wvp.xyww;
	varying_texcoord = vertex_position;
}