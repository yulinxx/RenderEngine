#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "Common/DllSet.h"
#include <QOpenGLFunctions_3_3_Core>
#include <QImage>
#include <QString>
#include <vector>

namespace GLRhi
{
    class GLRENDER_API TextureLoader
    {
    public:
        // 从文件加载纹理，返回 OpenGL 纹理 ID（0表示失败）
        static GLuint loadTextureFromFile(const QString& filePath,
            QOpenGLFunctions_3_3_Core* gl,
            bool flipY = true,
            GLint wrapMode = GL_CLAMP_TO_EDGE,
            GLint filterMode = GL_LINEAR);

        // 从 QImage 加载（Qt 原生方式，作为对比）
        static GLuint loadTextureFromImage(const QImage& image,
            QOpenGLFunctions_3_3_Core* gl,
            GLint wrapMode = GL_CLAMP_TO_EDGE,
            GLint filterMode = GL_LINEAR);

        // 创建纹理数组（从多个文件加载）
        static GLuint createTextureArray(const std::vector<QString>& imagePaths,
            QOpenGLFunctions_3_3_Core* gl,
            int width, int height,
            GLint wrapMode = GL_CLAMP_TO_EDGE,
            GLint filterMode = GL_LINEAR);

        // 删除纹理
        static void deleteTexture(GLuint textureId,
            QOpenGLFunctions_3_3_Core* gl);
    };
}

#endif