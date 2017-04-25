#version 450

in  vec3 varying_normal;
out vec4 frag_colour;

void main()
{
	frag_colour = vec4(varying_normal, 1.0);
}