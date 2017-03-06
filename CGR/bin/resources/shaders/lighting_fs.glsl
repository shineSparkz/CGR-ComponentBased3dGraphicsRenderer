#version 330
const int MAX_POINT_LIGHTS = 2;                          
const int MAX_SPOT_LIGHTS = 2;                                                   
                                                                                    
in vec2 varying_texcoord;                                                                
in vec3 varying_normal;                                                                    
in vec3 varying_position;                      
in vec3 varying_tangent;
in vec4 varying_light_position;                                            
                                                                                    
out vec4 frag_colour;                                                                 
                                                                                   
struct BaseLight                                                                    
{                                                                                   
    vec3 Color;                                                                     
    float AmbientIntensity;                                                         
    float DiffuseIntensity;                                                         
};                                                                                  
                                                                                    
struct DirectionalLight                                                             
{                                                                                   
    BaseLight Base;                                                                 
    vec3 Direction;                                                                 
};                                                                                  
                                                                                    
struct Attenuation                                                                  
{                                                                                   
    float Constant;                                                                 
    float Linear;                                                                   
    float Exp;                                                                      
};                                                                                  
                                                                                    
struct PointLight                                                                           
{                                                                                           
    BaseLight Base;                                                                         
    vec3 Position;                                                                          
    Attenuation Atten;                                                                      
};                                                                                          
                                                                                            
struct SpotLight                                                                            
{                                                                                           
    PointLight Base;                                                                        
    vec3 Direction;                                                                         
    float Cutoff;                                                                           
};                                                                                          
                                                                                            
uniform int u_NumPointLights;                                                                
uniform int u_NumSpotLights;                                                                 
uniform DirectionalLight u_DirectionalLight;                                                 
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];                                          
uniform SpotLight u_SpotLights[MAX_SPOT_LIGHTS];           
                                  
uniform sampler2D u_Sampler;     
uniform sampler2D u_ShadowSampler;              
uniform sampler2D u_NormalSampler;
                                              
uniform vec3 u_EyeWorldPos;                                                                  
uniform float u_MatSpecularIntensity;                                                        
uniform float u_SpecularPower;        

float CalcShadowFactor(vec4 lightSpacePos)
{
	vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
	
	vec2 uvCoords;
	uvCoords.x = 0.5 * projCoords.x + 0.5;
	uvCoords.y = 0.5 * projCoords.y + 0.5;
	float z = 0.5 * projCoords.z + 0.5;
	
	float depth = texture(u_ShadowSampler, uvCoords).x;
	if(depth < z + 0.00001)
		return 0.5;
	return 1.0;
}      
                                                                                                                                  
vec4 CalcLightInternal(BaseLight light, vec3 lightDirection, vec3 normal, float shadowFactor)                   
{                                                                                           
    vec4 ambientColor = vec4(light.Color * light.AmbientIntensity, 1.0f);
    float diffuseFactor = max(0.0, dot(normal, -lightDirection));                                     
                                                                                            
    vec4 diffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 specularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
    if (diffuseFactor > 0)
	{                                                                
        diffuseColor = vec4(light.Color * light.DiffuseIntensity * diffuseFactor, 1.0f);
                                                                                            
        vec3 vertexToEye = normalize(u_EyeWorldPos - varying_position);                             
        vec3 lightReflect = normalize(reflect(lightDirection, normal));                     
        float specularFactor = max(0.0,dot(vertexToEye, lightReflect));                                      
        
		if (specularFactor > 0)
		{                                                           
            specularFactor = pow(specularFactor, u_SpecularPower);                               
            specularColor = vec4(light.Color, 1.0f) * u_MatSpecularIntensity * specularFactor;
        }                                                                                   
    }                                                                                       
                                                                                            
    return (ambientColor + shadowFactor * diffuseColor + specularColor);                                   
}                                                                                           
                                                                                            
vec4 CalcDirectionalLight(vec3 normal)                                                      
{                                                                                           
    return CalcLightInternal(u_DirectionalLight.Base, u_DirectionalLight.Direction, normal, 1.0);  
}                                                                                           
                                                                                            
vec4 CalcPointLight(PointLight l, vec3 normal, vec4 lightSpacePos)                                              
{                                                                                           
    vec3 lightDirection = varying_position - l.Position;                                           	
	float dist = length(lightDirection);                                                
    lightDirection = normalize(lightDirection);  
	float shadowFactor = CalcShadowFactor(lightSpacePos);
                                                                                            
    vec4 colour = CalcLightInternal(l.Base, lightDirection, normal, shadowFactor);                         
	
	float attenuation = (l.Atten.Constant +                                                 
                        l.Atten.Linear * dist +                                        
                        l.Atten.Exp * dist * dist);                                 
                                                                                            
    return colour / attenuation;	
}                                                                                           
                                                                                            
vec4 CalcSpotLight(SpotLight l, vec3 normal, vec4 lightSpacePos)                                                
{                                                                                           
    vec3 lightToPixel = normalize(varying_position - l.Base.Position);                             
    float spotFactor = dot(lightToPixel, l.Direction);                                      
		
    if (spotFactor > l.Cutoff)
	{                                                            
        vec4 colour = CalcPointLight(l.Base, normal, lightSpacePos);                                        
        return colour * (1.0 - (1.0 - spotFactor) * 1.0/(1.0 - l.Cutoff));                   
    }                                                                                       
    else
	{                                                                                  
        return vec4(0,0,0,0);                                                               
    }                                                                                       
}                 

vec3 CalcBumpedNormal()
{
	vec3 normal = normalize(varying_normal);
	vec3 tangent = normalize(varying_tangent);
	
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 biTan = cross(tangent, normal);
	vec3 bumpNorm = texture(u_NormalSampler, varying_texcoord).xyz;
	
	bumpNorm = 2.0 * bumpNorm - vec3(1.0, 1.0, 1.0);                              
    vec3 newNormal;                                                                         
    mat3 TBN = mat3(tangent, biTan, normal);                                            
    newNormal = TBN * bumpNorm;                                                        
    newNormal = normalize(newNormal);                                                       
    return newNormal;   
}	
                                                                                            
void main()                                                                                 
{                                                                                           
    vec3 normal =  CalcBumpedNormal();// normalize(varying_normal);                                                       
    vec4 totalLight = CalcDirectionalLight(normal);                                         
           
    for (int i = 0 ; i < u_NumPointLights ; i++)
	{     
		totalLight += CalcPointLight(u_PointLights[i], normal, varying_light_position);
	}

    for (int i = 0 ; i < u_NumSpotLights ; i++)
	{                                            
        totalLight += CalcSpotLight(u_SpotLights[i], normal, varying_light_position);                                
    }     
	                                                                                        
    frag_colour = texture2D(u_Sampler, varying_texcoord.xy) * totalLight;                             
}
