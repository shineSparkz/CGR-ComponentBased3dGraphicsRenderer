#version 330

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_texcoord;

smooth out vec2 varying_texcoord;
smooth out vec3 varying_normal;
smooth out vec3 varying_position;

uniform mat4 u_WVPXform;
uniform mat4 u_WorldXform;
uniform mat4 u_HeightMapScaleXform;

void main()
{
    vec4 inPositionScaled = u_HeightMapScaleXform * vec4(vertex_position, 1.0);
	gl_Position = u_WVPXform * inPositionScaled;
	varying_texcoord = vertex_texcoord;
	varying_normal = (u_WorldXform * vec4(vertex_normal, 0.0)).xyz;
	vec4 vWorldPosLocal = u_WorldXform * inPositionScaled;
	varying_position = vWorldPosLocal.xyz;
}