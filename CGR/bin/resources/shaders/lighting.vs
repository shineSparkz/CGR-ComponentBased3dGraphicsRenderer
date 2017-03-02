#version 330                                                                        
                                                                                    
layout (location = 0) in vec3 vertex_position;                                             
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_texcoord;                                                                                            
                                                                                    
uniform mat4 u_WvpXform;                                                                  
uniform mat4 u_WorldXform;                                                                
                                                                                    
out vec2 varying_texcoord;                                                                 
out vec3 N;                                                                   
out vec3 P;                                                                 
                                                                                    
void main()                                                                         
{                                                                                   
    gl_Position = u_WvpXform * vec4(vertex_position, 1.0);                                       
    varying_texcoord = vertex_texcoord;                                                         
    N = (u_WorldXform * vec4(vertex_normal,   0.0)).xyz;                                 
    P = (u_WorldXform * vec4(vertex_position, 1.0)).xyz;                               
}