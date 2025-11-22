#include "Shader/PointShader.h"

const char* chPointVS = R"(
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aOffset;

uniform mat3 uCameraMat;
uniform mat3 uOffsetCamera;
uniform float uDepth;

void main()
{
    vec3 pos = vec3(aPos, 1.0) * uCameraMat;
    vec3 offset = uOffsetCamera * vec3(aOffset, 1.0) - uOffsetCamera * vec3(0.0, 0.0, 1.0);
    pos += offset;

    gl_Position = vec4(pos.xy, (1 - uDepth) / 2.0f, 1.0);
}

)";

const char* chPointFS = R"(
#version 330 core

uniform vec4 uColor;

out vec4 fragColor;

void main()
{
    fragColor = uColor;
}

)";