#version 450

layout (binding = 1, std140) uniform scene
{ 
	mat4 proj_xform;
	mat4 view_xform;
	vec3 camera_position;
	vec3 ambient_light;
	float delta_time;
};

layout (location = 0)   in  vec3    vertex_position;       	//!< The local position of the current vertex.
layout (location = 1)   in  vec3    vertex_normal;         	//!< The local normal vector of the current vertex.
layout (location = 2)   in  vec2    vertex_texcoord;       	//!< The texture co-ordinates for the vertex, used for mapping a texture to the object.
layout (location = 3) 	in 	vec3	vertex_tangent;

out vec3     N;
out vec3     P;
out vec2     varying_texcoord;   //!< The texture co-ordinate for the fragment to use for texture mapping.
out vec3	 varying_tangent;
out vec4 	 varying_light_position;

uniform mat4 u_world_xform;
uniform mat4 u_light_xform;

void main()
{
	gl_Position = proj_xform * view_xform * u_world_xform * vec4(vertex_position, 1.0);
	varying_texcoord = vertex_texcoord;
	varying_tangent = (u_world_xform * vec4(vertex_tangent, 0.0)).xyz;
	varying_light_position = u_light_xform * vec4(vertex_position, 1.0);
	
	N =  vec3( u_world_xform * vec4(vertex_normal,   0.0));
	P =  vec3( u_world_xform * vec4(vertex_position, 1.0));
}