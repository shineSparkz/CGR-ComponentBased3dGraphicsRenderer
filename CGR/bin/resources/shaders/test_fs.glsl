#version 330

out vec4 frag_colour;

uniform sampler2D texture_sampler;

//in vec3 varying_normal;
in vec2 varying_texcoord;

void main()
{
	//frag_colour = vec4(varying_normal, 1.0);
	frag_colour = texture(texture_sampler, varying_texcoord);
}