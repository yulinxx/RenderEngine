#include "Shader/LineUboShader.h"

const char* chLineUboVS = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in float aLineID;

uniform mat4 uCameraMatrix;

layout(std140) uniform LineDataUBO {
    vec4 colors[2048];
    float depths[2048];
};

flat out vec4 vColor;
out float vDepth;

void main()
{
    int id = int(aLineID);
    vColor = colors[id];
    vDepth = depths[id];
    gl_Position = uCameraMatrix * vec4(aPos.xy, (1 - vDepth) / 2.0f, 1.0);
}
)";

const char* chLineUboFS = R"(
#version 330 core
flat in vec4 vColor;
out vec4 fragColor;

void main()
{
    fragColor = vColor;
}
)";