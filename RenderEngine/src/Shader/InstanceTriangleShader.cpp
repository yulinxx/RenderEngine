#include "Shader/InstanceTriangleShader.h"

namespace GLRhi
{
    const char* instanceTriangleVS = R"(
        #version 330 core

        layout (location = 0) in vec3 aPos;

        layout (location = 1) in vec3 aInstancePos1;
        layout (location = 2) in vec3 aInstancePos2;
        layout (location = 3) in vec3 aInstancePos3;
        layout (location = 4) in vec4 aInstanceColor;
        layout (location = 5) in float aInstanceDepth;

        uniform mat4 uCameraMat;

        out vec4 vColor;

        void main()
        {
            // 根据基础顶点位置决定使用哪个实例顶点
            vec3 finalPos;
            if (aPos.x < -0.5) {
                finalPos = aInstancePos1;
            } else if (aPos.x > 0.5) {
                finalPos = aInstancePos2;
            } else {
                finalPos = aInstancePos3;
            }

            // 设置深度值
            finalPos.z = aInstanceDepth;

            // 应用相机矩阵
            gl_Position = vec4(finalPos, 1.0) * uCameraMat;

            // 传递颜色到片段着色器
            vColor = aInstanceColor;
        }
    )";

    const char* instanceTriangleFS = R"(
        #version 330 core

        in vec4 vColor;
        out vec4 FragColor;

        void main()
        {
            FragColor = vColor;
        }
    )";
}