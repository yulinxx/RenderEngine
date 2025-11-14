#include "Shader/BaseLineShader.h"

const char* baseLineVS = R"(
#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 uMVP;  // Model-View-Projection矩阵
uniform float uDepth = 0.0f;

void main()
{
    // 使用MVP矩阵进行坐标变换
    gl_Position = uMVP * vec4(aPos.xy, uDepth, 1.0);
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