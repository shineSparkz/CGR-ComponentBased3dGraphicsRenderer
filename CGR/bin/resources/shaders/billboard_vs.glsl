#version 330

layout (location = 0) in vec3 vertex_position;

void main()
{
	//mat4 scale = mat4(
	//	3.0, 0.0, 0.0, 0.0,
	//	0.0, 3.0, 0.0, 0.0,
	//	0.0, 0.0, 3.0, 0.0,
	//	0.0, 0.0, 0.0, 1.0);
		
	gl_Position = vec4(vertex_position, 1.0);
}