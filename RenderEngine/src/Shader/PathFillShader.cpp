#include "Shader/PathFillShader.h"

const char* chPathFillVS = R"(
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in float aLen;

uniform mat3 uCameraMat;
uniform float uDepth = 0.0;
uniform float uDashScale = 1.0;
uniform float uTimeOffset = 0.0;
uniform float uSpeed = 1.0;
uniform int uLineType = 0;

out float vDashParam;
flat out int vLineType;

void main()
{
    vec3 pos = vec3(aPos, 1.0) * uCameraMat;
    gl_Position = vec4(pos.xy, uDepth, 1.0);

    vLineType = uLineType;

    if(uLineType != 0)
    {
        float dashLength = aLen * uDashScale * 8.0 - (uTimeOffset * uSpeed);
        vDashParam = dashLength;
    }
    else
    {
        vDashParam = 0;
    }
}

)";

const char* chPathFillFS = R"(
#version 330 core

uniform vec4 uColor;

in float vDashParam;
flat in int vLineType;

out vec4 fragColor;

void main()
{
    if (vLineType != 0)
    {
        float dDutyCycle = 0.5;
        float dPeriod = 1.0;
        float dDashPattern = mod(vDashParam, dPeriod);
        bool bDraw = true;

        if (vLineType == 1)
        {
            float combinedPattern = mod(vDashParam * 2.0, 2.0);
            bDraw = combinedPattern < 1.4;
        }
        else if (vLineType == 2)
        {
            float combinedPattern = mod(vDashParam * 2, 5);  // 长
            bDraw = (combinedPattern < 3.5);
        }
        else if (vLineType == 3)
        {
            float combinedPattern = mod(vDashParam * 1.0, 4.0);   // 长 短 短
            bDraw = (combinedPattern < 1.6) || (combinedPattern >= 2.0 && combinedPattern < 2.6) || (combinedPattern >= 3 && combinedPattern < 3.6);
        }
        else if (vLineType == 4)     // 长长 点
        {
            float combinedPattern = mod(vDashParam, 3.0);
            bDraw = (combinedPattern < 1.5) || (combinedPattern >= 2.0 && combinedPattern < 2.2);
        }
        else if (vLineType == 5) // 长  点 点 点
        {
            float combinedPattern = mod(vDashParam, 3.0);
            bDraw = (0.2 < combinedPattern && combinedPattern < 1.8) || (combinedPattern >= 2.0 && combinedPattern < 2.2) ||
            (combinedPattern >= 2.4 && combinedPattern < 2.6) || (combinedPattern >= 2.8 && combinedPattern < 3.0);
        }
        else if (vLineType == 6)   // 长 点 点
        {
            float combinedPattern = mod(vDashParam, 3.0);
            bDraw = (combinedPattern < 1.4) || (combinedPattern >= 2.0 && combinedPattern < 2.2) ||
            (combinedPattern >= 2.4 && combinedPattern < 2.6);
        }
        else if (vLineType == 7)   // 点 点 点
        {
            float pattern = mod(vDashParam, 0.8);
            bDraw = (pattern < 0.2) || (pattern > 0.4 && pattern < 0.6);
        }
        else if (vLineType == 8) // 短长 点
        {
            float pattern = mod(vDashParam, 1.0);
            bDraw = (pattern < 0.6) || (pattern > 0.8 && pattern < 0.90);
        }
        else if (vLineType == 9)
        {
            float pattern = mod(vDashParam, 0.8);
            bDraw = (pattern < 0.1) || (pattern > 0.2 && pattern < 0.45);
        }
        else
        {
            bDraw = dDashPattern < dDutyCycle;
        }

        if (bDraw)
        {
            fragColor = uColor;
        }
        else
        {
            discard;
        }
    }
    else
    {
        fragColor = uColor;
    }
}

)";