#include "Shader/InstanceLineShader.h"

namespace GLRhi
{
    const char* instanceLineVS = R"(
        #version 330 core

        layout (location = 0) in vec3 aPos;

        layout (location = 1) in vec3 aInstancePos1;
        layout (location = 2) in vec3 aInstancePos2;
        layout (location = 3) in vec4 aInstanceColor;
        layout (location = 4) in float aInstanceWidth;
        layout (location = 5) in float aInstanceDepth;

        uniform mat4 uCameraMat;

        out vec4 vColor;

        void main()
        {
            // 计算线段方向
            vec3 lineDir = aInstancePos2 - aInstancePos1;
            float lineLength = length(lineDir);
            
            // 如果线段长度为0，直接返回
            if (lineLength == 0.0) {
                gl_Position = uCameraMat * vec4(aInstancePos1, 1.0);
                vColor = aInstanceColor;
                return;
            }

            // 归一化线段方向
            vec3 lineDirNorm = normalize(lineDir);
            
            // 计算线段的法线方向（垂直于线段的方向）
            vec3 normal = normalize(cross(lineDirNorm, vec3(0.0, 0.0, 1.0)));
            
            // 确定顶点位置和偏移方向
            vec3 finalPos;
            float offsetSign;
            
            if (aPos.x < 0.0) {
                // 使用第一个端点
                finalPos = aInstancePos1;
                // 根据y坐标决定偏移方向（上下两侧）
                offsetSign = (aPos.y < 0.0) ? -1.0 : 1.0;
            } else {
                // 使用第二个端点
                finalPos = aInstancePos2;
                // 根据y坐标决定偏移方向（上下两侧）
                offsetSign = (aPos.y < 0.0) ? -1.0 : 1.0;
            }
            
            // 应用宽度偏移
            finalPos += normal * offsetSign * aInstanceWidth * 0.5;
            
            // 设置深度值
            finalPos.z = aInstanceDepth;

            // 应用相机矩阵
            gl_Position = uCameraMat * vec4(finalPos, 1.0);

            // 传递颜色到片段着色器
            vColor = aInstanceColor;
        }
    )";

    const char* instanceLineFS = R"(
        #version 330 core

        in vec4 vColor;

        out vec4 FragColor;

        void main()
        {
            FragColor = vColor;
        }
    )";
}