#version 330                                                                        
                                                                                    
const int MAX_POINT_LIGHTS = 2;                                                     
const int MAX_SPOT_LIGHTS = 2;                                                      
                                                                                    
in vec2 varying_texcoord;                                                                  
in vec3 N;                                                                    
in vec3 P;                                                                  
                                                                                    
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
uniform vec3 u_EyeWorldPos;                                                                  
uniform float u_MatSpecularIntensity;                                                        
uniform float u_SpecularPower;                                                               
                                                                                            
vec4 CalcLightInternal(BaseLight light, vec3 lightDirection, vec3 normal)                   
{                                                                                           
    vec4 ambientColor = vec4(light.Color * light.AmbientIntensity, 1.0f);
    float diffuseFactor = dot(normal, -lightDirection);                                     
                                                                                            
    vec4 diffuseColor  = vec4(0, 0, 0, 0);                                                  
    vec4 specularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
    if (diffuseFactor > 0)
	{                                                                
        diffuseColor = vec4(light.Color * light.DiffuseIntensity * diffuseFactor, 1.0f);
                                                                                            
        vec3 vertexToEye = normalize(u_EyeWorldPos - P);                             
        vec3 lightReflect = normalize(reflect(lightDirection, normal));                     
        float specularFactor = dot(vertexToEye, lightReflect);                                      
        
		if (specularFactor > 0)
		{                                                           
            specularFactor = pow(specularFactor, u_SpecularPower);                               
            specularColor = vec4(light.Color * u_MatSpecularIntensity * specularFactor, 1.0f);
        }                                                                                   
    }                                                                                       
                                                                                            
    return (ambientColor + diffuseColor + specularColor);                                   
}                                                                                           
                                                                                            
vec4 CalcDirectionalLight(vec3 normal)                                                      
{                                                                                           
    return CalcLightInternal(u_DirectionalLight.Base, u_DirectionalLight.Direction, normal);  
}                                                                                           
                                                                                            
vec4 CalcPointLight(PointLight l, vec3 normal)                                              
{                                                                                           
    vec3 lightDirection = P - l.Position;                                           
    float distance = length(lightDirection);                                                
    lightDirection = normalize(lightDirection);                                             
                                                                                            
    vec4 colour = CalcLightInternal(l.Base, lightDirection, normal);                         
    float attenuation =  l.Atten.Constant +                                                 
                         l.Atten.Linear * distance +                                        
                         l.Atten.Exp * distance * distance;                                 
                                                                                            
    return colour / attenuation;                                                             
}                                                                                           
                                                                                            
vec4 CalcSpotLight(SpotLight l, vec3 normal)                                                
{                                                                                           
    vec3 lightToPixel = normalize(P - l.Base.Position);                             
    float spotFactor = dot(lightToPixel, l.Direction);                                      
                                                                                            
    if (spotFactor > l.Cutoff)
	{                                                            
        vec4 colour = CalcPointLight(l.Base, normal);                                        
        return colour * (1.0 - (1.0 - spotFactor) * 1.0/(1.0 - l.Cutoff));                   
    }                                                                                       
    else
	{                                                                                  
        return vec4(0,0,0,0);                                                               
    }                                                                                       
}                                                                                           
                                                                                            
void main()                                                                                 
{                                                                                           
    vec3 normal = normalize(N);                                                       
    vec4 totalLight = CalcDirectionalLight(normal);                                         
                                                                                            
    for (int i = 0 ; i < u_NumPointLights ; i++)
	{                                           
        totalLight += CalcPointLight(u_PointLights[i], normal);                              
    }                                                                                       
                                                                                            
    for (int i = 0 ; i < u_NumSpotLights ; i++)
	{                                            
        totalLight += CalcSpotLight(u_SpotLights[i], normal);                                
    }                                                                                       
                                                                                            
    frag_colour = texture2D(u_Sampler, varying_texcoord.xy) * totalLight;                             
}