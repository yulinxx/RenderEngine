#include "BaseTriangleShader.h"

// 着色器源码
const char* baseTriangleVS = R"(
#version 330 core
layout(location = 0) in vec3 position; // x, y, len

uniform mat4 cameraMat;

void main() {
    gl_Position = cameraMat * vec4(position.xy, 0.0, 1.0);
}


)";

const char* baseTriangleFS = R"(
#version 330 core
out vec4 fragColor;

uniform vec4 color;
uniform float depth;

void main() {
    fragColor = vec4(color.rgb, 1.0);
}
)";