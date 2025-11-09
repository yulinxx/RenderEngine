#ifndef FAKE_TEXTURE_DATA_H
#define FAKE_TEXTURE_DATA_H

#include <vector>
#include <random>
#include <QOpenGLFunctions_3_3_Core>

namespace GLRhi
{
    struct TextureData;

    class FakeTextureData final
    {
    public:
        FakeTextureData();
        ~FakeTextureData();

    public:
        // 设置XY的最小/最大范围
        void setRange(float xMin, float xMax, float yMin, float yMax);

        // 设置纹理尺寸范围
        void setTextureSizeRange(int minWidth, int maxWidth, int minHeight, int maxHeight);

        // 生成指定数量的纹理数据
        // count: 要生成的纹理数量
        // gl: OpenGL 函数对象，用于创建纹理
        void generateTextures(int count, QOpenGLFunctions_3_3_Core* gl);

        // 获取生成的纹理数据
        const std::vector<TextureData>& getTextureDatas() const;

        // 清空数据并释放纹理资源
        void clear(QOpenGLFunctions_3_3_Core* gl);

        // 生成随机浮点数
        float getRandomFloat(float min, float max);

        // 生成随机整数
        int getRandomInt(int min, int max);

    private:
        // 生成单个纹理数据
        void generateSingleTexture(QOpenGLFunctions_3_3_Core* gl);

        // 生成随机位图数据
        unsigned char* generateRandomImageData(int width, int height, int& channels);

    private:
        float m_xMin = -1.0f;           // X轴最小值
        float m_xMax = 1.0f;            // X轴最大值
        float m_yMin = -1.0f;           // Y轴最小值
        float m_yMax = 1.0f;            // Y轴最大值

        int m_minWidth = 32;            // 最小纹理宽度
        int m_maxWidth = 256;           // 最大纹理宽度
        int m_minHeight = 32;           // 最小纹理高度
        int m_maxHeight = 256;          // 最大纹理高度

        std::vector<TextureData> m_textureDatas; // 纹理数据
        std::vector<GLuint> m_textureIds;        // 存储纹理ID，用于清理

        // 随机数生成器
        std::mt19937 m_generator;
        std::random_device m_randomDevice;
    };
}

#endif // FAKE_TEXTURE_DATA_H