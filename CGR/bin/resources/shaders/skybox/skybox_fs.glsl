#version 330 core

in vec3 varying_texcoord;

uniform samplerCube cube_sampler;

out vec4 fragment_colour;

void main()
{
	fragment_colour = texture(cube_sampler, varying_texcoord);
}

