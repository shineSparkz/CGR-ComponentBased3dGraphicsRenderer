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
    float   aConstant;      
    float   aLinear;        
    float   aQuadratic;
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

vec4 getDirectionalLightColour(in vec3 n)
{
	float diffuse_intensity = max(0.0, dot(n, -directionLight.direction));
	float fMult = clamp(0.1 + diffuse_intensity, 0.0, 1.0);
	return vec4(directionLight.intensity * fMult, 1.0);
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
	
	frag_colour = vec4(ambient_light, colour.a) + getDirectionalLightColour(normal) * colour;
	//frag_colour = vec4(normal, 1.0);
}
