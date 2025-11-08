
#version 330 core

layout (location = 0) in vec3 in_pos;
  
uniform mat3 cameraTrans;  
uniform float depth;  
  
out vec4 vPosition ;
  
void main()  
{  
    vPosition  = vec4(in_pos, 1.0);
    gl_Position = vec4(in_pos, 1.0);
}