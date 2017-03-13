#version 330

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

out vec4 frag_colour;

uniform sampler2D u_PositionMap;
uniform sampler2D u_ColourMap;
uniform sampler2D u_NormalMap;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLight;
uniform SpotLight u_SpotLight;

uniform vec3 u_EyeWorldPos;
uniform float u_MatSpecularIntensity;
uniform float u_SpecularPower;
uniform int u_LightType;
uniform vec2 u_ScreenSize;

vec4 CalcLightInternal(BaseLight light,
					   vec3 lightDirection,
					   vec3 worldPos,
					   vec3 normal)
{
    vec4 ambientColor = vec4(light.Color * light.AmbientIntensity, 1.0);
    float diffuseFactor = dot(normal, -lightDirection);

    vec4 diffuseColor  = vec4(0, 0, 0, 0);
    vec4 specularColor = vec4(0, 0, 0, 0);

    if (diffuseFactor > 0.0) {
        diffuseColor = vec4(light.Color * light.DiffuseIntensity * diffuseFactor, 1.0);

        vec3 vertexToEye = normalize(u_EyeWorldPos - worldPos);
        vec3 lightReflect = normalize(reflect(lightDirection, normal));
        float specularFactor = dot(vertexToEye, lightReflect);        
        if (specularFactor > 0.0) {
            specularFactor = pow(specularFactor, u_SpecularPower);
            specularColor = vec4(light.Color * u_MatSpecularIntensity * specularFactor, 1.0);
        }
    }

    return (ambientColor + diffuseColor + specularColor);
}

vec4 CalcDirectionalLight(vec3 worldPos, vec3 normal)
{
    return CalcLightInternal(u_DirectionalLight.Base,
							 u_DirectionalLight.Direction,
							 worldPos,
							 normal);
}

vec4 CalcPointLight(vec3 worldPos, vec3 normal)
{
    vec3 lightDirection = worldPos - u_PointLight.Position;
    float distance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    vec4 colour = CalcLightInternal(u_PointLight.Base, lightDirection, worldPos, normal);

    float attenuation =  u_PointLight.Atten.Constant +
                         u_PointLight.Atten.Linear * distance +
                         u_PointLight.Atten.Exp * distance * distance;

    attenuation = max(1.0, attenuation);

    return colour / attenuation;
}


vec2 CalcTexCoord()
{
    return gl_FragCoord.xy / u_ScreenSize;
}

void main()
{
    vec2 texcoord = CalcTexCoord();
	vec3 worldPos = texture(u_PositionMap, texcoord).xyz;
	vec3 colour = texture(u_ColourMap, texcoord).xyz;
	vec3 normal = texture(u_NormalMap, texcoord).xyz;
	normal = normalize(normal);

	frag_colour = vec4(colour, 1.0) * CalcPointLight(worldPos, normal);
}
