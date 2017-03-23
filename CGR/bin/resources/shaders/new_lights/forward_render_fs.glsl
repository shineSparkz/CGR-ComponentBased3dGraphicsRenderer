#version 450

//#define MAX_SPOTS 10
#define MAX_POINTS 10

struct DirectionalLight
{
    vec3 direction; 
    vec3 intensity;
	float ambient_intensity;
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

uniform DirectionalLight u_dir_light;
uniform Spotlight u_spot_light;
uniform PointLight u_point_light[MAX_POINTS];

uniform vec3 u_ambience;
uniform sampler2D u_sampler;
uniform int u_num_point_lights;

in  vec3    N;
in  vec3 	P;
in  vec2    varying_texcoord;   //!< The interpolated co-ordinate to use for the texture sampler.
out vec4    frag_colour; //!< The calculated colour of the fragment.

vec4 getDirectionalLightColor(DirectionalLight dirLight, vec3 n)
{
	float diffuse_intensity = max(0.0, dot(n, -dirLight.direction));
	float fMult = clamp(dirLight.ambient_intensity + diffuse_intensity, 0.0, 1.0);
	return vec4(dirLight.intensity * fMult, 1.0);
}

vec4 getPointLightColor(const PointLight ptLight, vec3 p, vec3 n) 
{ 
   vec3 light_dir = p - ptLight.position; 
   float dist = length(light_dir); 
   light_dir = normalize(light_dir); 
    
   float diffuse = max(0.0, dot(n, -light_dir)); 
   float attenuation = ptLight.aConstant + ptLight.aLinear * dist + ptLight.aQuadratic * dist*dist; 
   return vec4(ptLight.intensity, 1.0) * (ptLight.ambient_intensity + diffuse) / attenuation; 
}

vec4 getSpotLightColor(const Spotlight spotLight, vec3 p)
{
	// If flashlight isn't turned on, return no color
	if(spotLight.switched_on == 0)
		return vec4(0.0, 0.0, 0.0, 0.0);
	
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
	
	vec4 total_light =  getDirectionalLightColor(u_dir_light, n) + 
						getSpotLightColor(u_spot_light, P);
						
	for(int i = 0; i < u_num_point_lights; ++i)
	{
		total_light += getPointLightColor(u_point_light[i], P, n);
	}
						
	frag_colour = vec4(u_ambience, tex_colour.a) + total_light * tex_colour;
}


