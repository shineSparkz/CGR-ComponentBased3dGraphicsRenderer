#version 330

layout (location = 0) in vec3 vertex_position; 

uniform mat4 u_WVP;

void main()
{          
    gl_Position = u_WVP * vec4(vertex_position, 1.0);
}
