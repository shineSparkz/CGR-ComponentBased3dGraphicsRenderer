#version 450

// Reference block to get access to these
layout (binding = 1, std140) uniform scene
{ 
	mat4 proj_xform;
	mat4 view_xform;
	vec3 camera_position;
	vec3 ambient_light;
	float delta_time;
};

// The standard input attributes for a mesh, but can be customised
layout (location = 0)   in  vec3    vertex_position;       	
layout (location = 1)   in  vec3    vertex_normal;         	
layout (location = 2)   in  vec2    vertex_texcoord;
layout (location = 3) 	in 	vec3	vertex_tangent;

// Pass these out to fragment shader
out vec3 varying_normal;

// Pipeline will auto pass this uniform for each mesh
uniform mat4 u_world_xform;

void main()
{
	gl_Position = (proj_xform * view_xform * u_world_xform) * vec4(vertex_position, 1.0);
	varying_normal = vertex_normal * 0.5 + vec3(0.5);	// <-- Keep between 0 and 1
}