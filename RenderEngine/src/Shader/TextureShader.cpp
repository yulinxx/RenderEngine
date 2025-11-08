#include "TextureShader.h"

const char* chTextureVS = R"(
#version 330 core

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;

uniform mat3 cameraMat;
uniform float depth;

out vec2 texCoords;

void main()
{
    vec3 v = cameraMat * vec3(in_pos, 1.0);
    gl_Position = vec4(v.xy, depth, 1.0);

    vec2 uvYRev = vec2(in_uv.x, 1 - in_uv.y);
    texCoords = uvYRev;
}
)";

const char* chTextureFS = R"(
#version 330 core

uniform sampler2D texSampler;
in vec2 texCoords;

out vec4 fragColor;

void main()
{
    fragColor = texture(texSampler, texCoords);
}
)";