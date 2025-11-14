#include "Shader/FreeExShader.h"

const char* chFreeExVS = R"(
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aOffset;
layout(location = 2) in vec4 aDepth;
layout(location = 3) in vec4 aColor;

uniform mat3 uOriCamera;
uniform mat3 uOffsetCamera;

out vec4 vOutColor;

void main()
{
    vec3 pos = uOriCamera * vec3(aPos, 1.0);
    vec3 offset = uOffsetCamera * vec3(aOffset, 1.0) - uOffsetCamera * vec3(0.0, 0.0, 1.0);

    pos += offset;

    //gl_Position  = vec4(pos.xy, aDepth.x, 1.0);
    gl_Position  = vec4(pos.xy, (1 - aDepth.x) / 2.0f, 1.0);
    vOutColor = aColor;
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