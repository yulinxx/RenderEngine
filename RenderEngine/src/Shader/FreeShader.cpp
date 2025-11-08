#include "FreeShader.h"

const char* chFreeVS = R"(
#version 330 core

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_color;

uniform mat3 camera;

out vec4 vColor;

void main()
{
    vec3 transformedPos = camera * vec3(in_pos.xy, 1.0);
    //gl_Position = vec4(transformedPos.xy, in_pos.z, 1.0);
    gl_Position = vec4(transformedPos.xy, (1 - in_pos.z) / 2.0f, 1.0);
    vColor = in_color;
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