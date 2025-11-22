#ifndef FAKE_TEXTURE_DATA_H
#define FAKE_TEXTURE_DATA_H

#include <vector>
#include <QOpenGLFunctions_3_3_Core>
#include "FakeData/FakeDataBase.h"

namespace GLRhi
{
    struct TextureData;

    class FakeTextureData : public FakeDataBase
    {
    public:
        FakeTextureData();
        ~FakeTextureData() override;

    public:
        void clear() override;

    public:
        // 设置纹理尺寸范围
        void setTextureSizeRange(int minWidth, int maxWidth, int minHeight, int maxHeight);

        // 生成指定数量的纹理数据
        // count: 要生成的纹理数量
        // gl: OpenGL 函数对象，用于创建纹理
        void generateTextures(int count, QOpenGLFunctions_3_3_Core* gl);

        // 获取生成的纹理数据
        const std::vector<TextureData>& getTextureDatas() const;

        void clearTexture(QOpenGLFunctions_3_3_Core* gl);

    private:
        // 生成单个纹理数据
        void generateSingleTexture(QOpenGLFunctions_3_3_Core* gl);

        // 生成随机位图数据
        unsigned char* generateRandomImageData(int width, int height, int& channels);

    private:
        int m_minWidth = 32;            // 最小纹理宽度
        int m_maxWidth = 256;           // 最大纹理宽度
        int m_minHeight = 32;           // 最小纹理高度
        int m_maxHeight = 256;          // 最大纹理高度

        std::vector<TextureData> m_textureDatas; // 纹理数据
        std::vector<GLuint> m_textureIds;        // 存储纹理ID，用于清理
    };
}

#endif // FAKE_TEXTURE_DATA_H