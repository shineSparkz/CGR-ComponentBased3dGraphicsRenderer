#version 450

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 12) out;

uniform float u_time;
uniform float u_scale = 4.0;
uniform mat4 u_view_xform;
uniform mat4 u_proj_xform;
uniform mat4 u_model_xform;

out vec2 geom_texcoord;
out vec3 geom_normal;
out vec3 geom_position;

vec3 g_local_seed;

// Function Definitions
mat4 	rotationMatrix(vec3 axis, float angle);
float 	randZeroOne();
int 	randomInt(int min, int max);

void main()
{
	mat4 wvp = u_proj_xform * u_view_xform * u_model_xform;
	vec3 billboard_pos = gl_in[0].gl_Position.xyz;

	float pIover180 = 3.1415/180.0;
	vec3 base_direciton[] =
	{
		vec3(1.0, 0.0, 0.0),
		vec3(float(cos(45.0*pIover180)), 0.0f, float(sin(45.0*pIover180))),
		vec3(float(cos(-45.0*pIover180)), 0.0f, float(sin(-45.0*pIover180)))
	};
	
	const float wind_strength = 2.0;
	
	vec3 wind_direction = vec3(1.0, 0.0, 1.0);
	wind_direction = normalize(wind_direction);
	
	for(int i = 0; i < 3; i++)
	{
		// Grass patch top left vertex
		vec3 base_dir_rotated = (rotationMatrix(vec3(0, 1, 0), sin(u_time * 0.7f) * 0.1f) * vec4(base_direciton[i], 1.0)).xyz;

		g_local_seed = billboard_pos * float(i);
		int grass_patch = randomInt(0, 3);
		
		float grass_patch_height = 3.5 + randZeroOne() * 2.0;
	
		float tex_start_x = float(grass_patch) * 0.25f;
		float tex_end_x = tex_start_x + 0.25f;
		
		float wind_power = 0.5f + sin(billboard_pos.x / 30 + billboard_pos.z / 30 + u_time * (1.2f + wind_strength / 20.0f));
		if(wind_power < 0.0f)
			wind_power = wind_power  * 0.2f;
		else 
			wind_power = wind_power * 0.3f;
		
		wind_power *= wind_strength;
		
		vec3 vTL = billboard_pos - base_dir_rotated * u_scale * 0.5f + wind_direction * wind_power;
		vTL.y += grass_patch_height;   
		gl_Position = wvp * vec4(vTL, 1.0);
		geom_texcoord = vec2(tex_start_x, 1.0);
		geom_position = vTL;
		EmitVertex();
		
		// Grass patch bottom left vertex
		vec3 vBL = billboard_pos - base_direciton[i] * u_scale * 0.5f;  
		gl_Position = wvp * vec4(vBL, 1.0);
		geom_texcoord = vec2(tex_start_x, 0.0);
		geom_position = vBL;
		EmitVertex();
		                               
		// Grass patch top right vertex
		vec3 vTR = billboard_pos + base_dir_rotated * u_scale * 0.5f + wind_direction * wind_power;
		vTR.y += grass_patch_height;  
		gl_Position = wvp * vec4(vTR, 1.0);
		geom_texcoord = vec2(tex_end_x, 1.0);
		geom_position = vTR;
		EmitVertex();
		
		// Grass patch bottom right vertex
		vec3 vBR = billboard_pos + base_direciton[i] * u_scale * 0.5f;  
		gl_Position = wvp * vec4(vBR, 1.0);
		geom_texcoord = vec2(tex_end_x, 0.0);
		geom_position = vBR;
		EmitVertex();
		
		EndPrimitive();
	}	
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

float randZeroOne()
{
	// This function returns random number from zero to one
    uint n = floatBitsToUint(g_local_seed.y * 214013.0 + g_local_seed.x * 2531011.0 + g_local_seed.z * 141251.0);
    n = n * (n * n * 15731u + 789221u);
    n = (n >> 9u) | 0x3F800000u;
 
    float fRes =  2.0 - uintBitsToFloat(n);
    g_local_seed = vec3(g_local_seed.x + 147158.0 * fRes, g_local_seed.y*fRes  + 415161.0 * fRes, g_local_seed.z + 324154.0*fRes);
    return fRes;
}

int randomInt(int min, int max)
{
	float fRandomFloat = randZeroOne();
	return int(float(min)+fRandomFloat*float(max-min));
}