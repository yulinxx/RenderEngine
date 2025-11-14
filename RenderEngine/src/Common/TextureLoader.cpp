#include "Common/TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include <QDebug>
#include <QFile>
#include <vector>
#include <cstring>

namespace GLRhi
{
    struct ImageData
    {
        std::vector<unsigned char> data;
        int width = 0;
        int height = 0;
        int channels = 0;
        bool isValid() const
        {
            return !data.empty() && width > 0 && height > 0;
        }
    };

    ImageData loadAndResizeImage(const QString& path, int nTargetWidth, int nTargetHeight)
    {
        ImageData imgData;

        if (!QFile::exists(path))
        {
            qWarning() << "Image file does not exist:" << path;
            return imgData;
        }

        QByteArray pathArray = path.toLocal8Bit();
        unsigned char* chImgData = stbi_load(pathArray.constData(),
            &imgData.width, &imgData.height, &imgData.channels, 4); // 强制RGBA

        if (!chImgData)
        {
            qWarning() << "STB Load failed:" << stbi_failure_reason() << "for" << path;
            return imgData;
        }

        imgData.data.resize(nTargetWidth * nTargetHeight * 4);

        if (imgData.width != nTargetWidth || imgData.height != nTargetHeight)
        {
            stbir_resize_uint8_linear(
                chImgData, imgData.width, imgData.height, 0,
                imgData.data.data(), nTargetWidth, nTargetHeight, 0,
                STBIR_RGBA
            );
            imgData.width = nTargetWidth;
            imgData.height = nTargetHeight;
        }
        else
        {
            std::memcpy(imgData.data.data(), chImgData, imgData.width * imgData.height * 4);
        }

        stbi_image_free(chImgData);
        return imgData;
    }

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
        GLenum internalFormat = GL_RGBA8;
        if (channels == 1)
        {
            format = GL_RED;
            internalFormat = GL_R8;
        }
        else if (channels == 3)
        {
            format = GL_RGB;
            internalFormat = GL_RGB8;
        }
        else if (channels == 4)
        {
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
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

        // 清理内存测试 - 加载后立即删除
        if (0)
        {
            gl->glDeleteTextures(1, &textureId);
            textureId = 0;
        }

        return textureId;
    }

    GLuint TextureLoader::loadTextureFromImage(const QImage& image,
        QOpenGLFunctions_3_3_Core* gl,
        GLint wrapMode,
        GLint filterMode)
    {
        if (image.isNull() || !gl)
            return 0;

        QImage glImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored();

        GLuint textureId = 0;
        gl->glGenTextures(1, &textureId);
        gl->glBindTexture(GL_TEXTURE_2D, textureId);

        // 使用GL_RGBA8内部格式
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
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

        int nLayerCount = static_cast<int>(imagePaths.size());
        GLuint texArray = 0;
        gl->glGenTextures(1, &texArray);
        gl->glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);

        gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height,
            nLayerCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapMode);
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapMode);
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filterMode);
        gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filterMode);

        std::vector<unsigned char> vErrorPlaceholder(width * height * 4);
        for (int p = 0; p < width * height; ++p)
        {
            vErrorPlaceholder[p * 4 + 0] = 255; // R
            vErrorPlaceholder[p * 4 + 1] = 0;   // G
            vErrorPlaceholder[p * 4 + 2] = 255; // B
            vErrorPlaceholder[p * 4 + 3] = 255; // A
        }

        stbi_set_flip_vertically_on_load(true);

        for (int i = 0; i < nLayerCount; ++i)
        {
            ImageData imageData = loadAndResizeImage(imagePaths[i], width, height);

            const unsigned char* uploadData = vErrorPlaceholder.data();
            if (imageData.isValid())
                uploadData = imageData.data.data();
            else
                qWarning() << "Failed to load texture for array layer" << i << ":" << imagePaths[i];

            // 上传数据
            gl->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
                width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, uploadData);
        }

        if (filterMode == GL_LINEAR_MIPMAP_LINEAR || filterMode == GL_NEAREST_MIPMAP_NEAREST)
        {
            gl->glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
            gl->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD, 4);
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
            // qDebug() << "Texture deleted, ID:" << textureId;
        }
    }
}