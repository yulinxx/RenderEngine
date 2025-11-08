#include "BaseLineShader.h"

// 着色器源码
const char* baseLineVS = R"(
#version 330 core

layout(location = 0) in vec3 aPos;

//uniform mat4 uCamera;
uniform float uDepth = 0.0f;

void main() {
    // gl_Position = uCamera * vec4(aPos.xy, uDepth, 1.0);
    gl_Position = vec4(aPos.xy, uDepth, 1.0);
}

)";

const char* baseLineFS = R"(
#version 330 core

out vec4 FragColor;

uniform vec4 uColor;

void main()
{
    FragColor = uColor;
}
)";