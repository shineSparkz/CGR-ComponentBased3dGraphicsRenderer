#version  330

smooth in vec2 varying_texcoord;
smooth in vec3 varying_normal;
smooth in vec3 varying_position;

out vec4 frag_colour;
                                                                               
struct directionalLight                                                             
{                                                                                                                                                                                             
    float ambientIntensity;                                                                      
    vec3 color;  	
	vec3 dir;
};  

uniform directionalLight u_DirectionalLight;
uniform sampler2D u_Sampler[5];
uniform sampler2D u_shadowMap;
uniform vec4 u_Colour;
uniform float u_RenderHeight;
uniform float u_MaxTexU;
uniform float u_MaxTexV;


vec4 GetDirectionalLightColor(vec3 vNormal)
{
	float fDiffuseIntensity = max(0.0, dot(vNormal, normalize(-u_DirectionalLight.dir)));
	float fMult = clamp(u_DirectionalLight.ambientIntensity + fDiffuseIntensity, 0.0, 1.0);
	return vec4(u_DirectionalLight.color * fMult, 1.0);
}

void main()
{
	vec3 vNormalized = normalize(varying_normal);
	vec4 vTexColor = vec4(0.0);
	float fScale = varying_position.y / u_RenderHeight;

	const float fRange1 = 0.15f;
	const float fRange2 = 0.3f;
	const float fRange3 = 0.65f;
	const float fRange4 = 0.85f;

	if(fScale >= 0.0 && fScale <= fRange1)
	{
		vTexColor = texture2D(u_Sampler[0], varying_texcoord);
	}
	else if(fScale <= fRange2)
	{
		fScale -= fRange1;
		fScale /= (fRange2-fRange1);
		
		float fScale2 = fScale;
		fScale = 1.0-fScale; 
		
		vTexColor += texture2D(u_Sampler[0], varying_texcoord)*fScale;
		vTexColor += texture2D(u_Sampler[1], varying_texcoord)*fScale2;
	}
	else if(fScale <= fRange3)
	{
		vTexColor = texture2D(u_Sampler[1], varying_texcoord);
	}
	else if(fScale <= fRange4)
	{
		fScale -= fRange3;
		fScale /= (fRange4-fRange3);
		
		float fScale2 = fScale;
		fScale = 1.0-fScale; 
		
		vTexColor += texture2D(u_Sampler[1], varying_texcoord)*fScale;
		vTexColor += texture2D(u_Sampler[2], varying_texcoord)*fScale2;		
	}
	else 
	{
		vTexColor = texture2D(u_Sampler[2], varying_texcoord);
	}

	vec2 vPathCoord = vec2(varying_texcoord.x / u_MaxTexU, varying_texcoord.y / u_MaxTexV);
	vec4 vPathIntensity = texture2D(u_Sampler[4], vPathCoord);
	fScale = vPathIntensity.x;
  
	vec4 vPathColor = texture2D(u_Sampler[3], varying_texcoord); // Black color means there is a path
	vec4 vFinalTexColor = fScale * vTexColor+(1-fScale)*vPathColor;

	vec4 vMixedColor = vFinalTexColor * u_Colour;
	vec4 vDirLightColor = GetDirectionalLightColor(vNormalized);

	frag_colour = vMixedColor*(vDirLightColor);  
}      