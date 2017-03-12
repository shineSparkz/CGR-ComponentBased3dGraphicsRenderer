#version 330

uniform sampler2D u_TextureMap;

in vec2 varying_texcoord;

out vec4 frag_colour;

void main()
{
	frag_colour = texture2D(u_TextureMap, varying_texcoord);
	
	if(frag_colour.a == 0)
	//if (frag_colour.r == 0 && frag_colour.g == 0 && frag_colour.b == 0)
	{
		discard;                                                                    
    }       
}