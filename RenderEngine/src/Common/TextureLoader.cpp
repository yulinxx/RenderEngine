#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <QDebug>
#include <QFile>

namespace GLRhi
{
    GLuint TextureLoader::loadTextureFromFile(const QString& filePath,
        QOpenGLFunctions_3_3_Core* gl,
        bool flipY,
        GLint wrapMode,
        GLint filterMode)
    {
        if (!gl || !QFile::exists(filePath))
        {
            qWarning() << "TextureLoader: Invalid GL or file not exists:" << filePath;
            return 0;
        }

        stbi_set_flip_vertically_on_load(flipY);

        int width, height, channels;
        QByteArray pathArray = filePath.toLocal8Bit();
        unsigned char* data = stbi_load(pathArray.constData(),
            &width, &height, &channels, 0);

        if (!data)
        {
            qWarning() << "STB Load failed:" << stbi_failure_reason();
            return 0;
        }

        GLenum format = GL_RGBA;
        GLenum internalFormat = GL_RGBA;
        if (channels == 1)
        {
            format = GL_RED;
            internalFormat = GL_RED;
        }
        else if (channels == 3)
        {
            format = GL_RGB;
            internalFormat = GL_RGB;
        }
        else if (channels == 4)
        {
            format = GL_RGBA;
            internalFormat = GL_RGBA;
        }

        GLuint textureId = 0;
        gl->glGenTextures(1, &textureId);
        gl->glBindTexture(GL_TEXTURE_2D, textureId);

        gl->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
            format, GL_UNSIGNED_BYTE, data);

        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);

        if (filterMode == GL_LINEAR_MIPMAP_LINEAR || filterMode == GL_NEAREST_MIPMAP_NEAREST)
        {
            gl->glGenerateMipmap(GL_TEXTURE_2D);
        }

        stbi_image_free(data);

        gl->glBindTexture(GL_TEXTURE_2D, 0);

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
    GLuint TextureLoader::createTextureArray(const std::vector<QString>& imagePaths,
        QOpenGLFunctions_3_3_Core* gl,
        int width, int height,
        GLint wrapMode, GLint filterMode)
    {
        if (imagePaths.empty() || !gl || width <= 0 || height <= 0)
        {
            qWarning() << "TextureLoader: Invalid input parameters";
            return 0;
        }

        int layerCount = imagePaths.size();
        GLuint texArray = 0;
        gl->glGenTextures(1, &texArray);
        gl->glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);

        // 分配存储空间（layerCount层）
        gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height,
            layerCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // 设置纹理参数
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapMode);
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapMode);
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filterMode);
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filterMode);

        // 逐层上传
        for (int i = 0; i < layerCount; ++i)
        {
            // 使用现有的loadTextureFromFile逻辑加载单个图像
            stbi_set_flip_vertically_on_load(true);
            int imgWidth, imgHeight, channels;
            QByteArray pathArray = imagePaths[i].toLocal8Bit();
            unsigned char* data = stbi_load(pathArray.constData(),
                &imgWidth, &imgHeight, &channels, 4); // 强制加载为RGBA

            if (data)
            {
                // 如果图像尺寸不匹配，需要进行缩放
                // 这里简化处理，直接上传
                gl->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
                    imgWidth, imgHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                qWarning() << "Failed to load texture for array layer" << i << ":" << imagePaths[i];
            }
        }

        // 如果需要Mipmap
        if (filterMode == GL_LINEAR_MIPMAP_LINEAR || filterMode == GL_NEAREST_MIPMAP_NEAREST)
        {
            gl->glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        }

        gl->glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        return texArray;
    }

    void TextureLoader::deleteTexture(GLuint textureId,
        QOpenGLFunctions_3_3_Core* gl)
    {
        if (textureId && gl)
        {
            gl->glDeleteTextures(1, &textureId);
            qDebug() << "Texture deleted, ID:" << textureId;
        }
    }
}