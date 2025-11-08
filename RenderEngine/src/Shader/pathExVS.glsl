#version 330 core

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_offset;

uniform mat3 cameraTrans;
uniform mat3 offsetCamera;
uniform float depth;

void main() 
{
    vec3 pos = cameraTrans * vec3(in_pos, 1.0);
    vec3 offset = offsetCamera * vec3(in_offset, 1.0) - offsetCamera * vec3(0.0, 0.0, 1.0);
    pos += offset;
    
    gl_Position = vec4(pos.xy, 0.2, 1.0);
}
