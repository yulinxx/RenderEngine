#include "FreeExShader.h"

const char* chFreeExVS = R"(
#version 330 core

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_offset;
layout(location = 2) in vec4 in_depth;
layout(location = 3) in vec4 in_color;

uniform mat3 oriCamera;
uniform mat3 offsetCamera;

out vec4 vOutColor;

void main()
{
    vec3 pos = oriCamera * vec3(in_pos, 1.0);
    vec3 offset = offsetCamera * vec3(in_offset, 1.0) - offsetCamera * vec3(0.0, 0.0, 1.0);

    pos += offset;

    //gl_Position  = vec4(pos.xy, in_depth.x, 1.0);
    gl_Position  = vec4(pos.xy, (1 - in_depth.x) / 2.0f, 1.0);
    vOutColor = in_color;
}
)";

const char* chFreeExFS = R"(
#version 330 core

in vec4 vOutColor;
out vec4 fragColor;

void main()
{
    fragColor = vOutColor;
}
)";