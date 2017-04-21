#version 450

layout (binding = 1, std140) uniform scene
{ 
	mat4 proj_xform;
	mat4 view_xform;
	vec3 camera_position;
	vec3 ambient_light;
	float delta_time;
};

layout(location=0) in vec3 vertex_position;
layout(location=1) in vec2 vertex_texcoord;
layout(location=2) in vec3 vertex_normal;
layout(location=3) in vec3 vertex_next_position;
layout(location=4) in vec3 vertex_next_normal;

out vec3 N;
out vec3 P;
out vec2 varying_texcoord;
out vec3 varying_tangent;
out vec4 varying_light_position;

uniform mat4 	u_world_xform;
uniform mat4 	u_light_xform;
uniform float 	u_lerp;

void main()
{
	mat4 model_view = view_xform * u_world_xform;  
	mat4 mvp = proj_xform * view_xform * u_world_xform;

	varying_texcoord = vertex_texcoord;

	vec3 lerp_pos = vertex_position;
	if(u_lerp >= 0.0f)
		lerp_pos += (vertex_next_position - vertex_position) * u_lerp;

	gl_Position = mvp * vec4(lerp_pos, 1.0);

	vec3 lerp_norm = vertex_normal;
	if(u_lerp >= 0.0f)
		lerp_norm += (vertex_next_normal - vertex_normal) * u_lerp;

	N =  vec3( u_world_xform * vec4(vertex_normal,   0.0));
	P =  vec3( u_world_xform * vec4(vertex_position, 1.0));
	
	varying_light_position = u_light_xform * vec4(vertex_position, 1.0);
	varying_tangent = vec3(0,1,0);
}