#version 330 core

out vec4 fragment_colour;

uniform sampler2D texture_sampler;

in vec3 varying_normal;
in vec2 varying_texcoord;

void main()
{
	//fragment_colour = vec4(1.0, 0.0, 0.0, 1.0);
	//fragment_colour = vec4(varying_normal, 1.0);
	fragment_colour = texture(texture_sampler, varying_texcoord);
}