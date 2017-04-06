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
};

// TODO : This will go in material block
uniform sampler2D u_sampler;

in  vec3    N;
in  vec3 	P;
in  vec2    varying_texcoord;   //!< The interpolated co-ordinate to use for the texture sampler.
out vec4    frag_colour; 		//!< The calculated colour of the fragment.

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

vec4 getDirectionalLightColour(in vec3 n)
{
	float diffuse_intensity = max(0.0, dot(n, -directionLight.direction));
	float fMult = clamp(0.1 + diffuse_intensity, 0.0, 1.0);
	return vec4(directionLight.intensity * fMult, 1.0);
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
}

vec4 getSpotLightColor(const Spotlight spotLight, vec3 p)
{
	// If flashlight isn't turned on, return no color
	if(spotLight.switched_on == 0)
		return vec4(0.0);
	
	// Distance from fragment's position
	float dist = distance(p, spotLight.position);
  
	// Get direction vector to fragment
	vec3 light_dir = p - spotLight.position;
	light_dir = normalize(light_dir);
  
	// Cosine between spotlight direction and directional vector to fragment
	float angle = dot(spotLight.direction, light_dir);
  
	// Difference between max cosine and current cosine
	float diff = 1.0 - spotLight.coneAngle;
  
	// This is how strong light is depending whether its nearer to the center of
	// cone or nearer to its borders (onway factor in article), clamp to 0.0 and 1.0
	float factor = clamp((angle - spotLight.coneAngle) / diff, 0.0, 1.0);
    
	// If we're inside cone, calculate color
	if(angle > spotLight.coneAngle)
		return vec4(spotLight.intensity, 1.0) * factor / (dist * spotLight.aLinear);
  
	// No color otherwise
	return vec4(0.0, 0.0, 0.0, 0.0);
}

void main()
{
	vec3 n = normalize(N);
	vec4 tex_colour = texture(u_sampler, varying_texcoord);
	
	vec4 total_light =  getDirectionalLightColour(n);
	
	for(int i = 0; i < numSpots; ++i)
	{
		total_light += getSpotLightColor(spotLights[i], P);
	}
						
	for(int i = 0; i < numPoints; ++i)
	{
		total_light += getPointLightColor(pointLights[i], P, n);
	}
						
	frag_colour = vec4(ambient_light, tex_colour.a) + total_light * tex_colour;
}


