#version 330

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 u_ViewProjXform;
uniform vec3 u_CamPos;
uniform float u_Scale = 1.0;

out vec2 geom_texcoord;
out vec3 geom_normal;
out vec3 geom_position;

void main()
{
	vec3 pos = gl_in[0].gl_Position.xyz;
	vec3 toCam = normalize(u_CamPos - pos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, toCam);
	
	//---------------------------------------
	
	pos -= (right * 0.5) * u_Scale;
	gl_Position = u_ViewProjXform * vec4(pos, 1.0);
	geom_texcoord = vec2(0.0, 0.0);
	geom_normal = toCam;
	geom_position = gl_Position.xyz;
	EmitVertex();
	
	pos.y += u_Scale;
	gl_Position = u_ViewProjXform * vec4(pos, 1.0);
	geom_texcoord = vec2(0.0, 1.0);
	geom_normal = toCam;
	geom_position = gl_Position.xyz;
	EmitVertex();
	
	pos.y -= u_Scale;
	pos += right * u_Scale;
	gl_Position = u_ViewProjXform * vec4(pos, 1.0);
	geom_texcoord = vec2(1.0, 0.0);
	geom_normal = toCam;
	geom_position = gl_Position.xyz;
	EmitVertex();
	
	pos.y += u_Scale;
	gl_Position = u_ViewProjXform * vec4(pos, 1.0);
	geom_texcoord = vec2(1.0, 1.0);
	geom_normal = toCam;
	geom_position = gl_Position.xyz;
	EmitVertex();
	
	EndPrimitive();
}