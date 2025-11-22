#include "Shader/BaseTriangleShader.h"

const char* baseTriangleVS = R"(
#version 330 core
layout(location = 0) in vec3 aPosition;

uniform mat4 uCameraMat;
uniform float uDepth;

void main()
{
    gl_Position = vec4(aPosition.xy, (1 - uDepth) / 2.0f, 1.0) * uCameraMat;
}
)";

const char* baseTriangleFS = R"(
#version 330 core

uniform vec4 uColor;

out vec4 fragColor;

void main()
{
    fragColor = uColor;
}
)";