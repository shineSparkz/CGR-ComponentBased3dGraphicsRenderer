#version 450

layout(points) in;
layout(line_strip, max_vertices = 2) out;

layout (binding = 1, std140) uniform scene
{ 
	mat4 proj_xform;
	mat4 view_xform;
	vec3 camera_position;
	vec3 ambient_light;
};

uniform mat4 u_world_xform;
//uniform mat4 u_wvp;
//uniform mat4 u_normal_xform;

in vec3 N[];

void main()
{
	const float normal_length = 15.0f;
	mat4 wvp = (proj_xform * view_xform * u_world_xform);
	
	vec3 n = (u_world_xform * vec4( N[0] * normal_length, 0.0)).xyz;
	//vec3 n = (u_normal_xform * vec4(N[0]*normal_length, 1.0)).xyz;
	vec3 p = gl_in[0].gl_Position.xyz;
	
	gl_Position = wvp * vec4(p, 1.0);
	EmitVertex();
	
	gl_Position = wvp * vec4(p + n, 1.0);
	EmitVertex();
	
	EndPrimitive();
}