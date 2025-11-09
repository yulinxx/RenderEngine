#include "TextureLoader.h"

// 必须在 implementation 文件里定义，且只能定义一次
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // 确保路径正确

#include <QDebug>
#include <QFile>

namespace Utils {

GLuint TextureLoader::loadTextureFromFile(const QString& filePath,
                                          QOpenGLFunctions_3_3_Core* gl,
                                          bool flipY,
                                          GLint wrapMode,
                                          GLint filterMode)
{
    if (!gl || !QFile::exists(filePath)) {
        qWarning() << "TextureLoader: Invalid GL or file not exists:" << filePath;
        return 0;
    }

    // 1. 加载图片数据
    stbi_set_flip_vertically_on_load(flipY);  // 是否翻转Y轴（OpenGL坐标系）
    
    int width, height, channels;
    QByteArray pathArray = filePath.toLocal8Bit();
    unsigned char* data = stbi_load(pathArray.constData(), 
                                    &width, &height, &channels, 0);
    
    if (!data) {
        qWarning() << "STB Load failed:" << stbi_failure_reason();
        return 0;
    }

    // 2. 确定 OpenGL 格式
    GLenum format = GL_RGBA;
    GLenum internalFormat = GL_RGBA;
    if (channels == 1) {
        format = GL_RED;
        internalFormat = GL_RED;
    } else if (channels == 3) {
        format = GL_RGB;
        internalFormat = GL_RGB;
    } else if (channels == 4) {
        format = GL_RGBA;
        internalFormat = GL_RGBA;
    }

    // 3. 生成 OpenGL 纹理
    GLuint textureId = 0;
    gl->glGenTextures(1, &textureId);
    gl->glBindTexture(GL_TEXTURE_2D, textureId);

    // 4. 上传数据
    gl->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                     format, GL_UNSIGNED_BYTE, data);

    // 5. 设置纹理参数
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);

    // 6. 生成Mipmaps（如果需要）
    if (filterMode == GL_LINEAR_MIPMAP_LINEAR || filterMode == GL_NEAREST_MIPMAP_NEAREST) {
        gl->glGenerateMipmap(GL_TEXTURE_2D);
    }

    // 7. 清理 stb 内存
    stbi_image_free(data);

    gl->glBindTexture(GL_TEXTURE_2D, 0);  // 解绑

    qDebug() << "Texture loaded:" << filePath 
             << "Size:" << width << "x" << height 
             << "Channels:" << channels 
             << "TextureID:" << textureId;

    return textureId;
}

GLuint TextureLoader::loadTextureFromImage(const QImage& image,
                                           QOpenGLFunctions_3_3_Core* gl,
                                           GLint wrapMode,
                                           GLint filterMode)
{
    if (image.isNull() || !gl) return 0;

    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored();

    GLuint textureId = 0;
    gl->glGenTextures(1, &textureId);
    gl->glBindTexture(GL_TEXTURE_2D, textureId);

    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     glImage.width(), glImage.height(), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, glImage.constBits());

    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);

    gl->glBindTexture(GL_TEXTURE_2D, 0);

    return textureId;
}

// void TextureRenderer::deleteTexture(GLuint textureId, 
//                                     QOpenGLFunctions_3_3_Core* gl)
// {
//     if (textureId && gl) {
//         gl->glDeleteTextures(1, &textureId);
//     }
// }

} // namespace Utils