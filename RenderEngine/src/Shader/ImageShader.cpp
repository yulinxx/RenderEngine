#include "Shader/ImageShader.h"

const char* chImageVS = R"(
#version 330 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 v_TexCoord;

uniform mat3 uCameraMat;
uniform float uDepth = 0.0;

void main() {
    vec3 pos = vec3(aPosition, (1.0 - uDepth) / 2.0);
    vec3 transformed = pos * uCameraMat;
    gl_Position = vec4(transformed.x, transformed.y, transformed.z, 1.0);

    v_TexCoord = aTexCoord;
}
)";

const char* chImageFS = R"(
#version 330 core

in vec2 v_TexCoord;

out vec4 fragColor;

uniform sampler2D uTex;

void main() {
    vec4 color = texture(uTex, v_TexCoord);
    fragColor = color;
}
)";