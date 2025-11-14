#include "Shader/UiTextureShader.h"

const char* chUiTextureVS = R"(

#version 330 core

uniform mat3 uOriCamera;
uniform mat3 uOffsetCamera;

uniform float uDepth;

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aOffset;
layout(location = 2) in vec2 aUv;

out vec2 fragUV;

void main()
{
    vec3 pos3 = vec3(aPos, 1.0);
    vec3 offset3 = vec3(aOffset, 1.0);
    vec3 zeroOffset3 = vec3(0.0, 0.0, 1.0);

    vec3 transformedPos = uOriCamera * pos3;
    vec3 transformedOffset = uOffsetCamera * offset3 - uOffsetCamera * zeroOffset3;

    transformedPos += transformedOffset;

    gl_Position = vec4(transformedPos.xy, (1 - uDepth) / 2.0f, 1.0);
    fragUV = aUv;
}

)";

const char* chUiTextureFS = R"(
#version 330 core

uniform vec4 uColor;
uniform sampler2D uTexSampler;

in vec2 fragUV;

out vec4 fragColor;

void main()
{
    float a = texture(uTexSampler, fragUV).r;
    fragColor = vec4(uColor.rgb, a);
}

)";