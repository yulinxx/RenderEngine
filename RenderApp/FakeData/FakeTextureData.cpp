#include "FakeData/FakeTextureData.h"
#include "Render/TextureRenderer.h"

#include <cstdlib>
#include <ctime>
#include <QDebug>

FakeTextureData::FakeTextureData()
{
    setRange(-1.0f, 1.0f, -1.0f, 1.0f);
}

FakeTextureData::~FakeTextureData()
{
    m_textureDatas.clear();
}

void FakeTextureData::setTextureSizeRange(int minWidth, int maxWidth, int minHeight, int maxHeight)
{
    m_minWidth = std::max(16, minWidth);
    m_maxWidth = std::max(m_minWidth, maxWidth);
    m_minHeight = std::max(16, minHeight);
    m_maxHeight = std::max(m_minHeight, maxHeight);
}

void FakeTextureData::generateTextures(int count, QOpenGLFunctions_3_3_Core* gl)
{
    if (count <= 0 || !gl)
        return;

    m_textureDatas.clear();
    m_textureDatas.shrink_to_fit();
    m_textureIds.clear();
    m_textureIds.shrink_to_fit();

    m_textureDatas.reserve(count);
    m_textureIds.reserve(count);

    for (int i = 0; i < count; ++i)
    {
        generateSingleTexture(gl);
    }
}

const std::vector<TextureData>& FakeTextureData::getTextureDatas() const
{
    return m_textureDatas;
}

void FakeTextureData::clear()
{
}

void FakeTextureData::clearTexture(QOpenGLFunctions_3_3_Core* gl)
{
    if (gl)
    {
        for (GLuint texId : m_textureIds)
        {
            if (texId > 0)
            {
                gl->glDeleteTextures(1, &texId);
            }
        }
    }

    m_textureIds.clear();
    m_textureDatas.clear();
}

void FakeTextureData::generateSingleTexture(QOpenGLFunctions_3_3_Core* gl)
{
    int width = getRandomInt(m_minWidth, m_maxWidth);
    int height = getRandomInt(m_minHeight, m_maxHeight);
    int channels = 4;

    GLuint textureId = 0;
    //for(int i = 0; i < 100; i++)
    {
        unsigned char* imageData = generateRandomImageData(width, height, channels);
        if (!imageData)
            return;

        gl->glGenTextures(1, &textureId);
        gl->glBindTexture(GL_TEXTURE_2D, textureId);

        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, imageData);

        // 生成Mipmap
        gl->glGenerateMipmap(GL_TEXTURE_2D);

        // 解绑纹理
        gl->glBindTexture(GL_TEXTURE_2D, 0);

        delete[] imageData;
    }

    float imgWidth = getRandomFloat(0.1f, 0.4f);
    float imgHeight = imgWidth * (static_cast<float>(height) / width);

    float maxX = m_xMax - imgWidth / 2;
    float minX = m_xMin + imgWidth / 2;
    float maxY = m_yMax - imgHeight / 2;
    float minY = m_yMin + imgHeight / 2;

    float centerX = getRandomFloat(minX, maxX);
    float centerY = getRandomFloat(minY, maxY);

    float left = centerX - imgWidth / 2;
    float right = centerX + imgWidth / 2;
    float bottom = centerY - imgHeight / 2;
    float top = centerY + imgHeight / 2;

    TextureData texData;

    // 顶点数据：x, y, u, v
    texData.vVerts = {
        left,  bottom, 0.0f, 0.0f,
        right, bottom, 1.0f, 0.0f,
        right, top,    1.0f, 1.0f,
        left,  top,    0.0f, 1.0f
    };

    texData.vIndices = { 0, 1, 2, 0, 2, 3 };

    texData.tex = textureId;

    texData.brush = {
        getRandomFloat(0.5f, 1.0f),  // r
        getRandomFloat(0.5f, 1.0f),  // g
        getRandomFloat(0.5f, 1.0f),  // b
        getRandomFloat(0.5f, 1.0f),  // a
        getRandomFloat(-1.0f, 1.0f), // depth
        0
    };

    m_textureDatas.push_back(texData);
    m_textureIds.push_back(textureId);
}

unsigned char* FakeTextureData::generateRandomImageData(int width, int height, int& channels)
{
    channels = 4;
    size_t sz = width * height * channels;
    unsigned char* data = new unsigned char[sz];

    int patternType = getRandomInt(0, 3); // 0-纯色, 1-棋盘格, 2-噪声, 3-渐变

    switch (patternType)
    {
    case 0:
    {
        unsigned char r = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char g = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char b = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char a = static_cast<unsigned char>(getRandomInt(128, 255));

        for (size_t i = 0; i < sz; i += 4)
        {
            // 添加索引检查，防止数组溢出
            if (i + 3 < sz)
            {
                data[i] = r;
                data[i + 1] = g;
                data[i + 2] = b;
                data[i + 3] = a;
            }
        }
        break;
    }
    case 1:
    {
        int checkSize = getRandomInt(4, 16);
        unsigned char r1 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char g1 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char b1 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char r2 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char g2 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char b2 = static_cast<unsigned char>(getRandomInt(50, 255));

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                size_t index = (static_cast<size_t>(y) * width + x) * 4;
                if (index + 3 < sz)
                {
                    bool useColor1 = ((x / checkSize) + (y / checkSize)) % 2 == 0;

                    data[index] = useColor1 ? r1 : r2;
                    data[index + 1] = useColor1 ? g1 : g2;
                    data[index + 2] = useColor1 ? b1 : b2;
                    data[index + 3] = 255;
                }
            }
        }
        break;
    }
    case 2:
    {
        for (size_t i = 0; i < sz; i += 4)
        {
            // 添加索引检查，防止数组溢出
            if (i + 3 < sz)
            {
                data[i] = static_cast<unsigned char>(getRandomInt(0, 255));
                data[i + 1] = static_cast<unsigned char>(getRandomInt(0, 255));
                data[i + 2] = static_cast<unsigned char>(getRandomInt(0, 255));
                data[i + 3] = 255;
            }
        }
        break;
    }
    case 3:
    {
        unsigned char r1 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char g1 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char b1 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char r2 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char g2 = static_cast<unsigned char>(getRandomInt(50, 255));
        unsigned char b2 = static_cast<unsigned char>(getRandomInt(50, 255));

        for (int y = 0; y < height; ++y)
        {
            float ratio = static_cast<float>(y) / height;
            for (int x = 0; x < width; ++x)
            {
                size_t index = (y * width + x) * 4;
                // 添加索引检查，防止数组溢出
                if (index + 3 < sz)
                {
                    data[index] = static_cast<unsigned char>(r1 + ratio * (r2 - r1));
                    data[index + 1] = static_cast<unsigned char>(g1 + ratio * (g2 - g1));
                    data[index + 2] = static_cast<unsigned char>(b1 + ratio * (b2 - b1));
                    data[index + 3] = 255;
                }
            }
        }
        break;
    }
    }

    return data;
}