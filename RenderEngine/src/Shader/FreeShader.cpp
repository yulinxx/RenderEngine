#include "Shader/FreeShader.h"

const char* chFreeVS = R"(
#version 330 core

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aColor;

uniform mat3 uCamera;

out vec4 vColor;

void main()
{
    vec3 transformedPos = uCamera * vec3(aPos.xy, 1.0);
    //gl_Position = vec4(transformedPos.xy, aPos.z, 1.0);
    gl_Position = vec4(transformedPos.xy, (1 - aPos.z) / 2.0f, 1.0);
    vColor = aColor;
}

)";

const char* chFreeFS = R"(
#version 330 core

in vec4 vColor;

out vec4 fragColor;

void main()
{
    fragColor = vColor;
}

)";