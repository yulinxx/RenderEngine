#include "Shader/BaseLineShader.h"

const char* baseLineVS = R"(
#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 uCameraMat;
uniform float uDepth = 0.0f;

void main()
{
    gl_Position = vec4(aPos.xy, uDepth, 1.0) * uCameraMat;
}

)";

const char* baseLineFS = R"(
#version 330 core

uniform vec4 uColor;
out vec4 FragColor;

void main()
{
    FragColor = uColor;
}
)";