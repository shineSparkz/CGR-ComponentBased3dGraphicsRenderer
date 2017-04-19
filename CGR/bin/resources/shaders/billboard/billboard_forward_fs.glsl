#version 450

uniform sampler2D u_TextureMap;

in vec2 geom_texcoord; 
in vec3 geom_normal; 
in vec3 geom_position; 

out vec4 frag_colour;

void main()
{		
	const float fAlphaMultiplier = 1.5;
	const float fAlphaTest = 0.25;
	
	vec4 vTexColor = texture2D(u_TextureMap, geom_texcoord);
	float fNewAlpha = vTexColor.a * fAlphaMultiplier;               
	if(fNewAlpha < fAlphaTest)
		discard;
	
	if(fNewAlpha > 1.0f)
		fNewAlpha = 1.0f;	
		
	vec4 vMixedColor = vTexColor * vec4(1.0); 
	
	frag_colour = vec4(vMixedColor.xyz, fNewAlpha);
}
