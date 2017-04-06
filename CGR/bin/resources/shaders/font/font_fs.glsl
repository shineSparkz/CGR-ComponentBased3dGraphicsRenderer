#version 330 core

layout(location = 0) out vec4 frag_colour;

in vec2 varying_texcoords;

uniform sampler2D text;
uniform vec4 text_colour;

void main()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, varying_texcoords).r);
	frag_colour = text_colour * sampled;
}