#include "FakeData/FakeDataProvider.h"

#include "FakeData/FakePolyLineData.h"
#include "FakeData/FakeTriangleData.h"
#include "FakeData/FakeTextureData.h"

#include "Common/TextureLoader.h"

#include <QDebug>
#include <QString>

namespace GLRhi
{
    FakeDataProvider::FakeDataProvider()
    {
    }

    FakeDataProvider::~FakeDataProvider()
    {
        cleanup();
    }

    void FakeDataProvider::initialize(QOpenGLFunctions_3_3_Core *gl)
    {
        m_gl = gl;
    }

    std::vector<PolylineData> FakeDataProvider::genLineData(size_t count /*=20*/)
    {
        std::vector<PolylineData> vPLineDatas;

        FakePolyLineData fakePlData;
        for (size_t i = 0; i < count; ++i)
        {
            fakePlData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
            fakePlData.generateLines(10, 2, 15);

            std::vector<float> datas = fakePlData.getVertices();
            std::vector<size_t> lineVertexCounts = fakePlData.getLineInfos();

            float r = fakePlData.getRandomFloat(0.0f, 1.0f);
            float g = fakePlData.getRandomFloat(0.0f, 1.0f);
            float b = fakePlData.getRandomFloat(0.0f, 1.0f);
            float a = fakePlData.getRandomFloat(0.1f, 1.0f);
            float d = fakePlData.getRandomFloat(-1.0f, 1.0f);

            PolylineData polyLineData{ m_idGenerator.genID(), datas, lineVertexCounts, {r, g, b, a, d} };
            vPLineDatas.emplace_back(polyLineData);
        }

        return vPLineDatas;
    }

    std::vector<TriangleData> FakeDataProvider::genTriangleData()
    {
        return genBlendTestTriangleData();
    }

    std::vector<TriangleData> FakeDataProvider::genBlendTestTriangleData()
    {
        std::vector<TriangleData> vTriDatas(4);

        // 红色，左上
        float dAlpha = FakeDataBase::getRandomFloat(0.2f, 1.0f);
        float dDepth = FakeDataBase::getRandomFloat(-1.0f, 1.0f);
        vTriDatas[0].verts = {-0.4f, 0.4f, -0.5f, 0.2f, 0.4f, -0.5f, -0.4f, -0.4f, -0.5f};
        vTriDatas[0].indices = {0, 1, 2};
        vTriDatas[0].brush = {1.0f, 0.0f, 0.0f, dAlpha, dDepth};

        // 绿色，右上
        dAlpha = FakeDataBase::getRandomFloat(0.2f, 1.0f);
        dDepth = FakeDataBase::getRandomFloat(-1.0f, 1.0f);
        vTriDatas[1].verts = {-0.2f, 0.4f, -0.6f, 0.4f, 0.4f, -0.6f, 0.4f, -0.4f, -0.6f};
        vTriDatas[1].indices = {0, 1, 2};
        vTriDatas[1].brush = {0.0f, 1.0f, 0.0f, dAlpha, dDepth};

        // 蓝色，左下
        dAlpha = FakeDataBase::getRandomFloat(0.2f, 1.0f);
        dDepth = FakeDataBase::getRandomFloat(-1.0f, 1.0f);
        vTriDatas[2].verts = {-0.4f, 0.0f, -0.7f, 0.2f, 0.0f, -0.7f, -0.4f, -0.6f, -0.7f};
        vTriDatas[2].indices = {0, 1, 2};
        vTriDatas[2].brush = {0.0f, 0.0f, 1.0f, dAlpha, dDepth};

        // 黄色，右下
        dAlpha = FakeDataBase::getRandomFloat(0.2f, 1.0f);
        dDepth = FakeDataBase::getRandomFloat(-1.0f, 1.0f);
        vTriDatas[3].verts = {-0.2f, 0.0f, -0.8f, 0.4f, 0.0f, -0.8f, 0.4f, -0.6f, -0.8f};
        vTriDatas[3].indices = {0, 1, 2};
        vTriDatas[3].brush = {1.0f, 1.0f, 0.0f, dAlpha, dDepth};
        return vTriDatas;
    }

    std::vector<TriangleData> FakeDataProvider::genRandomTriangleData()
    {
        std::vector<TriangleData> vTriDatas;
        vTriDatas.reserve(10);

        for (size_t i = 0; i < 10; ++i)
        {
            FakeTriangleData fakeTriangleData;
            fakeTriangleData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
            fakeTriangleData.generateTriangles(10);

            TriangleData triData{};
            triData.verts = fakeTriangleData.getVertices();
            triData.indices = fakeTriangleData.getIndices();

            float r = fakeTriangleData.getRandomFloat(0.0f, 1.0f);
            float g = fakeTriangleData.getRandomFloat(0.0f, 1.0f);
            float b = fakeTriangleData.getRandomFloat(0.0f, 1.0f);
            float a = fakeTriangleData.getRandomFloat(0.3f, 1.0f);
            float d = fakeTriangleData.getRandomFloat(-1.0f, 1.0f);
            triData.brush = {r, g, b, a, d};

            vTriDatas.push_back(triData);
        }

        return vTriDatas;
    }

    std::vector<TextureData> FakeDataProvider::genTextureData()
    {
        return genFileTextureData();
    }

    std::vector<TextureData> FakeDataProvider::genFileTextureData()
    {
        std::vector<TextureData> vTexDatas;

        if (!m_gl)
        {
            qWarning() << "FakeDataProvider: OpenGL functions not available";
            return vTexDatas;
        }

        QString imagePathA = "C:/Users/yulin/Pictures/a.jpg";
        QString imagePathB = "D:/tes t.测试+-文件/img/a.jpg";
        QString imagePathC = "D:/tes t.测试+-文件/img/ele2.png";
        QString imagePathD = "D:/tes t.测试+-文件/img/gg.png";

        QStringList imagePaths = {imagePathA, imagePathB, imagePathC, imagePathD};
        // QStringList imagePaths = { imagePathA };
        for (const QString &imagePath : imagePaths)
        {
            GLuint textureId = GLRhi::TextureLoader::loadTextureFromFile(
                imagePath, m_gl, true, GL_CLAMP_TO_EDGE, GL_LINEAR);

            if (textureId > 0)
            {
                TextureData texData;

                float x1 = FakeDataBase::getRandomFloat(-0.8f, 0.8f);
                float y1 = FakeDataBase::getRandomFloat(-0.8f, 0.8f);
                float x2 = FakeDataBase::getRandomFloat(-0.8f, 0.8f);
                float y2 = FakeDataBase::getRandomFloat(-0.8f, 0.8f);
                float x3 = FakeDataBase::getRandomFloat(-0.8f, 0.8f);
                float y3 = FakeDataBase::getRandomFloat(-0.8f, 0.8f);
                float x4 = FakeDataBase::getRandomFloat(-0.8f, 0.8f);
                float y4 = FakeDataBase::getRandomFloat(-0.8f, 0.8f);

                texData.verts = {
                    x1, y1, 0.0f, 0.0f,
                    x2, y2, 1.0f, 0.0f,
                    x3, y3, 1.0f, 1.0f,
                    x4, y4, 0.0f, 1.0f};

                texData.indices = {0, 1, 2, 0, 2, 3};

                texData.tex = textureId;

                float dAlpha = FakeDataBase::getRandomFloat(0.2f, 1.0f);
                float dDepth = FakeDataBase::getRandomFloat(-1.0f, 1.0f);

                texData.brush = {1.0f, 1.0f, 1.0f, dAlpha, dDepth};

                vTexDatas.push_back(texData);
            }
        }

        qWarning() << "FakeDataProvider: genFileTextureData OK";

        return vTexDatas;
    }

    std::vector<TextureData> FakeDataProvider::genRandomTextureData()
    {
        std::vector<TextureData> vTexDatas;

        if (!m_gl)
        {
            qWarning() << "FakeDataProvider: OpenGL functions not available";
            return vTexDatas;
        }

        FakeTextureData fakeTextureData;
        fakeTextureData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
        fakeTextureData.setTextureSizeRange(32, 256, 32, 256);
        fakeTextureData.generateTextures(10, m_gl);

        vTexDatas = fakeTextureData.getTextureDatas();

        return vTexDatas;
    }

    std::vector<InstanceTexData> FakeDataProvider::genInstanceTextureData(GLuint &tex, int &count)
    {
        std::vector<InstanceTexData> vInstances;

        if (!m_gl)
        {
            qWarning() << "FakeDataProvider: OpenGL functions not available";
            return vInstances;
        }

        QStringList imagePaths = {
            "C:/Users/yulin/Pictures/a.jpg",
            "D:/tes t.测试+-文件/img/a.jpg",
            "D:/tes t.测试+-文件/img/aaa18.jpg",
            "D:/tes t.测试+-文件/img/avatar.png"};

        QVector<QString> qVector = imagePaths.toVector();
        std::vector<QString> vImagePath(qVector.begin(), qVector.end());
        count = static_cast<int>(vImagePath.size());

        if (!vImagePath.empty())
        {
            tex = GLRhi::TextureLoader::createTextureArray(
                vImagePath,
                m_gl,
                256, // width
                256, // height
                GL_CLAMP_TO_EDGE,
                GL_LINEAR);

            if (tex > 0)
            {
                float dSpacing = 0.3f;
                float dSz = 0.2f;
                int nCols = 5;

                for (int i = 0; i < 20; ++i)
                {
                    InstanceTexData instance{};

                    // 坐标随机排列
                    instance.x = FakeDataBase::getRandomFloat(-0.9f, 0.9f);
                    instance.y = FakeDataBase::getRandomFloat(-0.9f, 0.9f);

                    // 尺寸随机化
                    instance.width = dSz * FakeDataBase::getRandomFloat(0.8f, 1.2f);
                    instance.height = dSz * FakeDataBase::getRandomFloat(0.8f, 1.2f);

                    instance.textureLayer = i % vImagePath.size();

                    // 透明度随机
                    instance.alpha = FakeDataBase::getRandomFloat(0.2f, 0.8f);

                    vInstances.push_back(instance);
                }
            }
            else
            {
                qWarning() << "FakeDataProvider: Failed to create texture array.";
            }
        }
        else
        {
            qWarning() << "FakeDataProvider: No valid image paths provided.";
        }

        return vInstances;
    }

    void FakeDataProvider::cleanup()
    {
        m_gl = nullptr;
    }
}