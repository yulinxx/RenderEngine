#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <QOpenGLFunctions_3_3_Core>
#include <QImage>
#include <QString>

namespace Utils {
    class TextureLoader {
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
        
        // 删除纹理
        // void deleteTexture(GLuint textureId, 
        //                           QOpenGLFunctions_3_3_Core* gl);
    };
}

#endif