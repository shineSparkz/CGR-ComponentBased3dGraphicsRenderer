#version 330

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_texcoord;

uniform mat4 model_xform;
uniform mat4 view_xform;
uniform mat4 proj_xform;

//out vec3 varying_normal;
out vec2 varying_texcoord;

void main()
{
	// Build Mat
	mat4 mvp = proj_xform * view_xform * model_xform;

	// Pass through tex coord
	varying_texcoord = vertex_texcoord;
	
	// Calc normals
	//varying_normal = vertex_normal;
	//varying_normal *= 0.5;
	//varying_normal += 0.5;
	
	// Set position
	gl_Position = mvp * vec4(vertex_position, 1.0);
}