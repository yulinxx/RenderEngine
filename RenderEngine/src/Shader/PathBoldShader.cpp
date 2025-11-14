#include "Shader/PathBoldShader.h"

const char* chPathBoldVS = R"(
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
    vec3 pos = uCameraMat * vec3(aPos, 1.0);
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

const char* chPathBoldGS = R"(
#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;
uniform float uThickness = 0.0015;

in float vDashParam[];
flat in int vLineType[];
out float gDashParam;
flat out int gLineType;

void main()
{
    vec2 p0 = gl_in[0].gl_Position.xy;
    vec2 p1 = gl_in[1].gl_Position.xy;
    float z = gl_in[0].gl_Position.z;

    vec2 dir = normalize(p1 - p0);
    vec2 normal = vec2(-dir.y, dir.x);
    vec2 offset = normal * uThickness;
    //vec2 offset(0.0f, 0.0f);

    gLineType = vLineType[0]; // 传递 vLineType
    gDashParam = vDashParam[0];
    gl_Position = vec4(p0 - offset, z, 1.0);
    EmitVertex();

    gLineType = vLineType[0];
    gDashParam = vDashParam[0];
    gl_Position = vec4(p0 + offset, z, 1.0);
    EmitVertex();

    gLineType = vLineType[1];
    gDashParam = vDashParam[1];
    gl_Position = vec4(p1 - offset, z, 1.0);
    EmitVertex();

    gLineType = vLineType[1];
    gDashParam = vDashParam[1];
    gl_Position = vec4(p1 + offset, z, 1.0);
    EmitVertex();
    EndPrimitive();
}

)";

const char* chPathBoldFS = R"(
#version 330 core
uniform vec4 uColor;

in float gDashParam;
flat in int gLineType;

out vec4 fragColor;

void main()
{
    if (gLineType != 0)
    {
        float dDutyCycle = 0.5;
        float dPeriod = 1.0;
        float dDashPattern = mod(gDashParam, dPeriod);
        bool bDraw = true;

        if (gLineType == 1)
        {
            float combinedPattern = mod(gDashParam * 2.0, 2.0);
            bDraw = combinedPattern < 1.4;
        }
        else if (gLineType == 2)
        {
            float combinedPattern = mod(gDashParam * 2, 5);  // 长 短
            bDraw = (combinedPattern < 3.5);
        }
        else if (gLineType == 3)
        {
            float combinedPattern = mod(gDashParam * 1.0, 4.0);   // 长 短 短
            bDraw = (combinedPattern < 1.6) || (combinedPattern >= 2.0 && combinedPattern < 2.6) || (combinedPattern >= 3 && combinedPattern < 3.6);
        }
        else if (gLineType == 4)     // 长 点
        {
            float combinedPattern = mod(gDashParam, 3.0);
            bDraw = (combinedPattern < 1.5) || (combinedPattern >= 2.0 && combinedPattern < 2.2);
        }
        else if (gLineType == 5) // 长  点 点 点
        {
            float combinedPattern = mod(gDashParam, 3.0);
            bDraw = (0.2 < combinedPattern && combinedPattern < 1.8) || (combinedPattern >= 2.0 && combinedPattern < 2.2) ||
            (combinedPattern >= 2.4 && combinedPattern < 2.6) || (combinedPattern >= 2.8 && combinedPattern < 3.0);
        }
        else if (gLineType == 6)   // 短 点 点
        {
            float combinedPattern = mod(gDashParam, 3.0);
            bDraw = (combinedPattern < 1.4) || (combinedPattern >= 2.0 && combinedPattern < 2.2) ||
            (combinedPattern >= 2.4 && combinedPattern < 2.6);
        }
        else if (gLineType == 7)   // 点 点 点
        {
            float pattern = mod(gDashParam, 0.8);
            bDraw = (pattern < 0.2) || (pattern > 0.4 && pattern < 0.6);
        }
        else if (gLineType == 8)
        {
            float pattern = mod(gDashParam, 1.0);
            bDraw = (pattern < 0.6) || (pattern > 0.8 && pattern < 0.90);
        }
        else if (gLineType == 9)
        {
            float pattern = mod(gDashParam, 0.8);
            bDraw = (pattern < 0.1) || (pattern > 0.2 && pattern < 0.45);
        }
        else
        {
            bDraw = dDashPattern < dDutyCycle;
        }

        if (bDraw)
        {
            fragColor = uColor;
            //fragColor = vec4(1.0, 1.0, 0.0, 1.0);
        }
        else
        {
            discard;
        }
    }
    else
    {
        fragColor = uColor;
        //fragColor = vec4(1.0, 1.0, 0.0, 1.0);
    }
}

)";