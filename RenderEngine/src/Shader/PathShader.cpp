#include "PathShader.h"

const char* chPathVS = R"(
#version 330 core

layout (location = 0) in vec2 in_pos;
layout (location = 1) in float in_len;

uniform mat3 cameraMat;
uniform float depth = 0.0;
uniform float dashScale = 1.0;
uniform float timeOffset = 0.0;
uniform float speed = 1.0;
uniform int lineType = 0;

out float dashParam;
flat out int o_lineType;

void main()
{
    vec3 pos = cameraMat * vec3(in_pos, 1.0);
    gl_Position = vec4(pos.xy, depth, 1.0);

    o_lineType = lineType;

    if(lineType != 0)
    {
        float dashLength = in_len * dashScale * 8.0 - (timeOffset * speed);
        dashParam = dashLength;
    }
    else
    {
        dashParam = 0;
    }
}
)";

const char* chPathFS = R"(
#version 330 core

uniform vec4 color;

in float dashParam;
flat in int o_lineType;

out vec4 fragColor;

void main()
{
    if (o_lineType != 0)
    {
        float dDutyCycle = 0.5;
        float dPeriod = 1.0;
        float dDashPattern = mod(dashParam, dPeriod);
        bool bDraw = true;

        if (o_lineType == 1)
        {
            float combinedPattern = mod(dashParam * 2.0, 2.0);
            bDraw = combinedPattern < 1.4;
        }
        else if (o_lineType == 2)
        {
            float combinedPattern = mod(dashParam * 2, 5);  // 长
            bDraw = (combinedPattern < 3.5);
        }
        else if (o_lineType == 3)
        {
            float combinedPattern = mod(dashParam * 1.0, 4.0);   // 长 短 短
            bDraw = (combinedPattern < 1.6) || (combinedPattern >= 2.0 && combinedPattern < 2.6) || (combinedPattern >= 3 && combinedPattern < 3.6);
        }
        else if (o_lineType == 4)     // 长长 点
        {
            float combinedPattern = mod(dashParam, 3.0);
            bDraw = (combinedPattern < 1.5) || (combinedPattern >= 2.0 && combinedPattern < 2.2);
        }
        else if (o_lineType == 5) // 长  点 点 点
        {
            float combinedPattern = mod(dashParam, 3.0);
            bDraw = (0.2 < combinedPattern && combinedPattern < 1.8) || (combinedPattern >= 2.0 && combinedPattern < 2.2) ||
            (combinedPattern >= 2.4 && combinedPattern < 2.6) || (combinedPattern >= 2.8 && combinedPattern < 3.0);
        }
        else if (o_lineType == 6)   // 长 点 点
        {
            float combinedPattern = mod(dashParam, 3.0);
            bDraw = (combinedPattern < 1.4) || (combinedPattern >= 2.0 && combinedPattern < 2.2) ||
            (combinedPattern >= 2.4 && combinedPattern < 2.6);
        }
        else if (o_lineType == 7)   // 点 点 点
        {
            float pattern = mod(dashParam, 0.8);
            bDraw = (pattern < 0.2) || (pattern > 0.4 && pattern < 0.6);
        }
        else if (o_lineType == 8) // 短长 点
        {
            float pattern = mod(dashParam, 1.0);
            bDraw = (pattern < 0.6) || (pattern > 0.8 && pattern < 0.90);
        }
        else if (o_lineType == 9)
        {
            float pattern = mod(dashParam, 0.8);
            bDraw = (pattern < 0.1) || (pattern > 0.2 && pattern < 0.45);
        }
        else
        {
            bDraw = dDashPattern < dDutyCycle;
        }

        if (bDraw)
        {
            fragColor = color;
        }
        else
        {
            discard;
        }
    }
    else
    {
        fragColor = color;
    }
}
)";