#version 330

in vec2 varying_texcoord;

uniform sampler2D u_shadow_sampler;

out vec4 frag_colour;

void main()
{
	float depth = texture(u_shadow_sampler, varying_texcoord).x;
	depth = 1.0 - (1.0 - depth) * 25.0;
	frag_colour = vec4(depth);
}