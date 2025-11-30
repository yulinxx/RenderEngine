#include "Shader/LineUboShader.h"

const char* chLineUboVS = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in float aLineID;

uniform mat4 uCameraMat;

//std140：内存布局规则，确保跨着色器的数据对齐一致性
//std140 是OpenGL中Uniform Block（统一块）的内存布局规则，
//用于明确指定着色器中Uniform变量在GPU内存中的排列方式，
// 确保CPU端与GPU端数据对齐一致
layout(std140) uniform uLineDataUBO {
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
    gl_Position = vec4(aPos.xy, (1 - vDepth) / 2.0f, 1.0) * uCameraMat;
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