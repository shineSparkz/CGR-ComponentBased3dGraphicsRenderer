#version 450

layout (binding = 1, std140) uniform scene
{ 
	mat4 proj_xform;
	mat4 view_xform;
	vec3 camera_position;
	vec3 ambient_light;
};

uniform mat4 u_world_xform;
uniform mat4 u_height_map_scale_xform;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_texcoord;

out vec2 varying_texcoord;
out vec3 varying_normal;
out vec3 varying_position;

void main()
{
    vec4 inPositionScaled = u_height_map_scale_xform * vec4(vertex_position, 1.0);
	gl_Position = proj_xform * view_xform * u_world_xform * inPositionScaled;
	varying_texcoord = vertex_texcoord;
	varying_normal = (u_world_xform * vec4(vertex_normal, 0.0)).xyz;
	vec4 vWorldPosLocal = u_world_xform * inPositionScaled;
	varying_position = vWorldPosLocal.xyz;
}