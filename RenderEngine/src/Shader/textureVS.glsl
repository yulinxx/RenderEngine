#version 330 core

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;

uniform mat3 cameraTrans;
uniform float depth;

out vec2 texCoords;

void main()
{
    vec3 v = cameraTrans * vec3(in_pos, 1.0);
    gl_Position = vec4(v.xy, depth, 1.0);
    
    vec2 uvYRev = vec2(in_uv.x, in_uv.y);
    texCoords = uvYRev;
}
