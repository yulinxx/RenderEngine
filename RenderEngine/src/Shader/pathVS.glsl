
#version 330 core

layout (location = 0) in vec2 in_pos;
  
uniform mat3 cameraTrans;  
uniform float depth;  
  
out vec4 posH;
  
void main()  
{  
    vec3 transformed = cameraTrans * vec3(in_pos, 1.0);  
    posH = vec4(transformed.xy, depth, 1.0);

    gl_Position = posH;
}