#version 330

uniform struct Matrices
{
	mat4 projXform;
	mat4 modelXform;
	mat4 viewXform;
} matrices;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_texcoord;

smooth out vec2 varying_texcoord;
smooth out vec3 varying_normal;
smooth out vec3 varying_position;

uniform mat4 u_HeightMapScaleXform;

void main()
{
    vec4 inPositionScaled = u_HeightMapScaleXform * vec4(vertex_position, 1.0);
	mat4 mMVP = matrices.projXform * matrices.viewXform * matrices.modelXform;
   
	gl_Position = mMVP * inPositionScaled;
  
	varying_texcoord = vertex_texcoord;
	varying_normal = vertex_normal;
   
	vec4 vWorldPosLocal = matrices.modelXform * inPositionScaled;
	varying_position = vWorldPosLocal.xyz;
}