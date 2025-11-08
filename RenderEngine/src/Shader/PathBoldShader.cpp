#include "PathBoldShader.h"

const char* chPathBoldVS = R"(
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

const char* chPathBoldGS = R"(
#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;
uniform float thickness = 0.0015;

in float dashParam[];
flat in int o_lineType[];
out float g_dashParam;
flat out int o_fs_lineType;

void main()
{
    vec2 p0 = gl_in[0].gl_Position.xy;
    vec2 p1 = gl_in[1].gl_Position.xy;
    float z = gl_in[0].gl_Position.z;

    vec2 dir = normalize(p1 - p0);
    vec2 normal = vec2(-dir.y, dir.x);
    vec2 offset = normal * thickness;
    //vec2 offset(0.0f, 0.0f);

    o_fs_lineType = o_lineType[0]; // 传递 o_lineType
    g_dashParam = dashParam[0];
    gl_Position = vec4(p0 - offset, z, 1.0);
    EmitVertex();

    o_fs_lineType = o_lineType[0];
    g_dashParam = dashParam[0];
    gl_Position = vec4(p0 + offset, z, 1.0);
    EmitVertex();

    o_fs_lineType = o_lineType[1];
    g_dashParam = dashParam[1];
    gl_Position = vec4(p1 - offset, z, 1.0);
    EmitVertex();

    o_fs_lineType = o_lineType[1];
    g_dashParam = dashParam[1];
    gl_Position = vec4(p1 + offset, z, 1.0);
    EmitVertex();
    EndPrimitive();
}

)";

const char* chPathBoldFS = R"(
#version 330 core
uniform vec4 color;

in float g_dashParam;
flat in int o_fs_lineType;

out vec4 fragColor;

void main()
{
    if (o_fs_lineType != 0)
    {
        float dDutyCycle = 0.5;
        float dPeriod = 1.0;
        float dDashPattern = mod(g_dashParam, dPeriod);
        bool bDraw = true;

        if (o_fs_lineType == 1)
        {
            float combinedPattern = mod(g_dashParam * 2.0, 2.0);
            bDraw = combinedPattern < 1.4;
        }
        else if (o_fs_lineType == 2)
        {
            float combinedPattern = mod(g_dashParam * 2, 5);  // 长 短
            bDraw = (combinedPattern < 3.5);
        }
        else if (o_fs_lineType == 3)
        {
            float combinedPattern = mod(g_dashParam * 1.0, 4.0);   // 长 短 短
            bDraw = (combinedPattern < 1.6) || (combinedPattern >= 2.0 && combinedPattern < 2.6) || (combinedPattern >= 3 && combinedPattern < 3.6);
        }
        else if (o_fs_lineType == 4)     // 长 点
        {
            float combinedPattern = mod(g_dashParam, 3.0);
            bDraw = (combinedPattern < 1.5) || (combinedPattern >= 2.0 && combinedPattern < 2.2);
        }
        else if (o_fs_lineType == 5) // 长  点 点 点
        {
            float combinedPattern = mod(g_dashParam, 3.0);
            bDraw = (0.2 < combinedPattern && combinedPattern < 1.8) || (combinedPattern >= 2.0 && combinedPattern < 2.2) ||
            (combinedPattern >= 2.4 && combinedPattern < 2.6) || (combinedPattern >= 2.8 && combinedPattern < 3.0);
        }
        else if (o_fs_lineType == 6)   // 短 点 点
        {
            float combinedPattern = mod(g_dashParam, 3.0);
            bDraw = (combinedPattern < 1.4) || (combinedPattern >= 2.0 && combinedPattern < 2.2) ||
            (combinedPattern >= 2.4 && combinedPattern < 2.6);
        }
        else if (o_fs_lineType == 7)   // 点 点 点
        {
            float pattern = mod(g_dashParam, 0.8);
            bDraw = (pattern < 0.2) || (pattern > 0.4 && pattern < 0.6);
        }
        else if (o_fs_lineType == 8)
        {
            float pattern = mod(g_dashParam, 1.0);
            bDraw = (pattern < 0.6) || (pattern > 0.8 && pattern < 0.90);
        }
        else if (o_fs_lineType == 9)
        {
            float pattern = mod(g_dashParam, 0.8);
            bDraw = (pattern < 0.1) || (pattern > 0.2 && pattern < 0.45);
        }
        else
        {
            bDraw = dDashPattern < dDutyCycle;
        }

        if (bDraw)
        {
            fragColor = color;
            //fragColor = vec4(1.0, 1.0, 0.0, 1.0);
        }
        else
        {
            discard;
        }
    }
    else
    {
        fragColor = color;
        //fragColor = vec4(1.0, 1.0, 0.0, 1.0);
    }
}

)";