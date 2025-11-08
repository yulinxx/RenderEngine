#include "PointShader.h"

const char* chPointVS = R"(
#version 330 core

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_offset;

uniform mat3 cameraMat;
uniform mat3 offsetCamera;
uniform float depth;

void main()
{
    vec3 pos = cameraMat * vec3(in_pos, 1.0);
    vec3 offset = offsetCamera * vec3(in_offset, 1.0) - offsetCamera * vec3(0.0, 0.0, 1.0);
    pos += offset;

    gl_Position = vec4(pos.xy, depth, 1.0);
}
)";

const char* chPointFS = R"(
#version 330 core

uniform vec4 color;

out vec4 fragColor;

void main()
{
    fragColor = color;
}
)";