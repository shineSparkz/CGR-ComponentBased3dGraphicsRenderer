#version 450

#define MAX_SPOTS 10
#define MAX_POINTS 10

struct DirectionLight
{
	vec3 direction;
	vec3 intensity;
};

struct Spotlight
{
    vec3    position;      
	vec3    direction;     
	vec3    intensity;      
    float   coneAngle;       
    float   aConstant;      //!< The constant co-efficient for the attenuation formula.
    float   aLinear;        //!< The linear co-efficient for the attenuation formula.
    float   aQuadratic;     //!< The quadratic co-efficient for the attenuation formula.
	int 	switched_on;
};

struct PointLight
{
    vec3    position;
    vec3    intensity;
	float   ambient_intensity;
    float   aConstant;
    float   aLinear;
    float   aQuadratic;
};

layout(binding = 0, std140) uniform Lights
{
	DirectionLight directionLight;
	PointLight pointLights[MAX_POINTS];
	Spotlight spotLights[MAX_SPOTS];
	int numSpots;
	int numPoints;
};

layout (binding = 1, std140) uniform scene
{ 
	mat4 proj_xform;
	mat4 view_xform;
	vec3 camera_position;
	vec3 ambient_light;
	float delta_time;
};

out vec4 frag_colour;

uniform sampler2D u_PositionMap;
uniform sampler2D u_ColourMap;
uniform sampler2D u_NormalMap;
uniform vec2 u_ScreenSize;
uniform int u_LightIndex;

vec4 reflection(vec3 light_intensity, float light_ambient_intensity, vec3 light_dir, vec3 p, vec3 n)
{
	vec4 ambColour = vec4(light_intensity * light_ambient_intensity, 1.0);
	vec4 difColour = vec4(0.0);
	vec4 specularColor = vec4(0, 0, 0, 0);
	
	float diffCoeff = dot(n, -light_dir);
	
	if(diffCoeff > 0)
	{
		// TODO : Make these part of meshes material
		const float specular_intensity = 0.2;
		const float shininess = 16.0;
		
		difColour = vec4(light_intensity * diffCoeff, 1.0);
		
		vec3 vertexToEye = normalize(camera_position - p);
        vec3 lightReflect = normalize(reflect(light_dir, n));
        float specularFactor = dot(vertexToEye, lightReflect);  
		
        if (specularFactor > 0.0) 
		{
            specularFactor = pow(specularFactor, shininess);
            specularColor = vec4(light_intensity * specular_intensity * specularFactor, 1.0);
        }
	}
	
	return (ambColour + difColour + specularColor);
}

vec4 getPointLightColor(const PointLight ptLight, vec3 p, vec3 n) 
{ 
   vec3 light_dir = p - ptLight.position; 
   float dist = length(light_dir); 
   // TODO : have this for each light source in struct
   const float light_range = 129.0f;	
   
   if(dist < light_range)
   {
	   light_dir = normalize(light_dir); 
	   vec4 colour = reflection(ptLight.intensity, ptLight.ambient_intensity, light_dir, p, n);
	   float attenuation = max(1.0, ptLight.aConstant + ptLight.aLinear * dist + ptLight.aQuadratic * dist*dist); 
	   return colour / attenuation;
   }
   
   return vec4(0.0);
   //return vec4(ptLight.intensity, 1.0) * (ptLight.ambient_intensity + diffuse) / attenuation; 
}

vec2 CalcTexCoord()
{
    return gl_FragCoord.xy / u_ScreenSize;
}

void main()
{
    vec2 texcoord = CalcTexCoord();
	vec3 worldPos = texture(u_PositionMap, texcoord).xyz;
	vec4 colour = texture(u_ColourMap, texcoord);
	vec3 normal = texture(u_NormalMap, texcoord).xyz;
	normal = normalize(normal);

	frag_colour = getPointLightColor(pointLights[u_LightIndex], worldPos, normal) * colour;
}
