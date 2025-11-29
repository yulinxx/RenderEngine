#include "FakeData/FakeDataProvider.h"

#include "FakeData/FakePolyLineData.h"
#include "FakeData/FakeTriangleData.h"

//#include "FakeData/PolylineRenderTest.h" // 包含测试头文件

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

    void FakeDataProvider::initialize()
    {
    }

    std::vector<PolylineData> FakeDataProvider::genLineData(
        size_t group /*=20*/, size_t nLineSz /*=100*/,  size_t minPts /*=2*/, size_t maxPts /*=10*/)
    {
        std::vector<PolylineData> vPLineDatas;

        FakePolyLineData fakePlData;
        size_t nVerts = 0;
        for (size_t i = 0; i < group; ++i)
        {
            fakePlData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
            fakePlData.generateLines(static_cast<int>(nLineSz), minPts, maxPts);
            //fakePlData.generateLines(nLineSz, 2, 2);

            std::vector<float> vVerts = fakePlData.getVertices();
            nVerts += (vVerts.size() / 3);

            std::vector<size_t> vLineVertexCounts = fakePlData.getLineInfos();

            std::vector<long long> vIDs;
            for (auto& line : vLineVertexCounts)
                vIDs.push_back(m_idGenerator.genID());

            Color c = fakePlData.genRandomColor();

            //qDebug() << "颜色: " << c.r() << ", " << c.g() << ", " << c.b() << ", " << c.a();

            //float d = fakePlData.getRandomFloat(-1.0f, 1.0f);

            PolylineData polyLineData{ vIDs, vLineVertexCounts, vVerts, {c} };
            vPLineDatas.emplace_back(polyLineData);
        }

        //qDebug() << "生成线段组数量: " << group << ", 每个组线段数量: " << nLineSz
        //    << ", 总线段数量: " << group * nLineSz << ", 总顶点数量: " << nVerts;

        return vPLineDatas;
    }

    void FakeDataProvider::disturbLineData(std::vector<PolylineData>& vPolylineDatas)
    {
        if (vPolylineDatas.empty())
            return;

        std::random_device rd;
        std::mt19937 rng(rd());

        // 1. 随机删除N%的图元
        if (0)
        {
            std::uniform_real_distribution<double> ratio(0.0, 0.3);
            double dDelRatio = ratio(rng);
            qDebug() << "删除比例: " << dDelRatio * 100 << "%";

            // 遍历每个线段组
            for (size_t i = 0; i < vPolylineDatas.size(); ++i)
            {
                auto& plData = vPolylineDatas[i];
                size_t nLineCount = plData.vCount.size();

                if (nLineCount == 0)
                    continue;

                // 计算需要删除的线段数量
                size_t nRemoveLines = static_cast<size_t>(nLineCount * dDelRatio);
                if (nRemoveLines > 0)
                {
                    // 创建需要保留的线段索引列表
                    std::vector<bool> vKeepLine(nLineCount, true);
                    size_t nRemoveCount = 0;

                    // 随机选择要删除的线段
                    while (nRemoveCount < nRemoveLines)
                    {
                        std::uniform_int_distribution<size_t> lineIndex(0, nLineCount - 1);
                        size_t nRemoveIndex = lineIndex(rng);

                        if (vKeepLine[nRemoveIndex])
                        {
                            vKeepLine[nRemoveIndex] = false;
                            nRemoveCount++;
                        }
                    }

                    // 构建新的顶点和计数数组
                    std::vector<float> vNewVerts;
                    std::vector<size_t> vNewCount;

                    // 计算每个顶点在verts数组中的起始位置
                    size_t nVertexOffset = 0;
                    for (size_t j = 0; j < nLineCount; ++j)
                    {
                        if (vKeepLine[j])
                        {
                            // 保留这条线段，将其顶点数据添加到新数组
                            size_t nVertexCount = plData.vCount[j] * 3; // 每个顶点有3个分量

                            vNewVerts.insert(vNewVerts.end(),
                                plData.vVerts.begin() + nVertexOffset,
                                plData.vVerts.begin() + nVertexOffset + nVertexCount);

                            vNewCount.push_back(plData.vCount[j]);
                        }
                        nVertexOffset += plData.vCount[j] * 3; // 移动到下一条线段的起始位置
                    }

                    // 更新线段组的数据
                    plData.vVerts = std::move(vNewVerts);
                    plData.vCount = std::move(vNewCount);
                }
            }
        }

        // 2. 修改30%的图元的顶点及颜色数据
        if (1)
        {
            std::uniform_real_distribution<double> prob_dist(0.0, 0.9);
            const double MODIFY_PROBABILITY = 0.3; // 30%的修改概率

            qDebug() << "修改概率: " << MODIFY_PROBABILITY * 100 << "%";

            // 遍历每个线段组
            for (size_t i = 0; i < vPolylineDatas.size(); ++i)
            {
                auto& plData = vPolylineDatas[i]; // 线段组数据

                // 对于选中的组，修改其顶点数据

                // 按线段进行遍历
                size_t nVertexStart = 0;
                for (size_t nLineIndex = 0; nLineIndex < plData.vCount.size(); ++nLineIndex)
                {
                    // 判断是否修改当前线段
                    if (prob_dist(rng) <= MODIFY_PROBABILITY)
                    {
                        // 修改当前线段的所有顶点位置
                        auto& vert = plData.vVerts[nVertexStart];

                        // 遍历当前线段的所有顶点
                        float centerX = 0.0f;
                        float centerY = 0.0f;

                        size_t nVertexCount = plData.vCount[nLineIndex];

                        size_t nOffset = nVertexStart;
                        for (int v = 0; v < nVertexCount; ++v)
                        {
                            centerX += plData.vVerts[nOffset++];
                            centerY += plData.vVerts[nOffset++];
                            nOffset++; // 跳过z分量
                        }

                        centerX /= nVertexCount;
                        centerY /= nVertexCount;

                        std::uniform_real_distribution<float> offset_distX(centerX - 0.1f, centerX + 0.1f);
                        std::uniform_real_distribution<float> offset_distY(centerY - 0.1f, centerY + 0.1f);

                        nOffset = nVertexStart;
                        for (int v = 0; v < nVertexCount; v++)
                        {
                            float x = offset_distX(rng);
                            float y = offset_distY(rng);

                            x = std::clamp(x, -0.98f, 0.98f);
                            y = std::clamp(y, -0.98f, 0.98f);

                            plData.vVerts[nOffset++] = x;
                            plData.vVerts[nOffset++] = y;

                            nOffset++; // 跳过z分量
                        }
                    }

                    // 更新顶点索引到下一条线段
                    nVertexStart += plData.vCount[nLineIndex];
                }

                // 同时修改该组的颜色
                FakePolyLineData fakePlData;
                Color c = fakePlData.genRandomColor();
                plData.brush = { c };
            }
        }

        // 3. 添加新数据 - 创建不固定数量的新线段，但确保总数不超过五百万
        if (1)
        {
            // 设置最大值
            const size_t MAX_TOTAL_COUNT = 5000000;

            // 计算还可以添加的最大数量（随机添加1到10条，但不超过最大值限制）
            size_t currentCount = vPolylineDatas.size();
            size_t availableSlots = MAX_TOTAL_COUNT > currentCount ? MAX_TOTAL_COUNT - currentCount : 0;

            if (availableSlots > 0)
            {
                // 生成随机数量的线段组，每组包含N条线段
                // std::uniform_int_distribution<size_t> group(0, 10);
                // std::uniform_int_distribution<size_t> lines(0, currentCount);

                std::vector<PolylineData> vPLineDatas = genLineData(1, 1);
                vPolylineDatas.insert(vPolylineDatas.end(), vPLineDatas.begin(), vPLineDatas.end());
            }
            else
            {
                qDebug() << "已达到最大线段数量限制 (" << MAX_TOTAL_COUNT << ")，无法添加新线段";
            }
        }
    }

    void FakeDataProvider::disturbLineDataVBO()
    {
        std::random_device rd;
        std::mt19937 rng(rd());

        // 0. 添加随机线段组
        if (1)
        {
            std::vector<PolylineData> vPLineDatas = genLineData(1, 4);

            m_vPolylineDatas.insert(m_vPolylineDatas.end(), vPLineDatas.begin(), vPLineDatas.end());

            m_plVboManager.addPolylines(vPLineDatas);

            // for (auto& pl : vPLineDatas)
            //{
            //     // 对于每条PolylineData中的每个线段，添加为单条多段线
            //     for (size_t i = 0; i < pl.vId.size(); ++i)
            //     {
            //        // 从顶点数组中提取当前线段的顶点数据
            //        size_t startIdx = 0;
            //        for (size_t j = 0; j < i; ++j)
            //        {
            //            startIdx += pl.vCount[j] * 3; // 每个顶点3个分量
            //        }
            //        size_t vertexCount = pl.vCount[i];
            //        std::vector<float> vertices(pl.vVerts.begin() + startIdx,
            //                                  pl.vVerts.begin() + startIdx + vertexCount * 3);
            //
            //        // 使用addPolyline方法添加单条多段线
            //        m_plVboManager.addPolyline(pl.vId[i], vertices,
            //            Color(pl.brush.r(), pl.brush.g(), pl.brush.b(), pl.brush.a()));
            //    }
            //}
        }

        // 1. 随机删除N%的图元
        if (1)
        {
            std::uniform_real_distribution<double> ratio(0.0, 0.3);
            double dDelRatio = ratio(rng);
            //qDebug() << "删除比例: " << dDelRatio * 100 << "%";

            for (auto& plData : m_vPolylineDatas)
            {
                for (auto& id : plData.vId)
                {
                    if (id % 3 == 0)
                    {
                        qDebug() << "删除ID: " << id;
                        m_plVboManager.removePolyline(id);
                    }
                }
            }
        }

        // 2. 修改30%的图元的顶点及颜色数据
        if (0)
        {
            // bool PolylinesVboManager::updatePolyline(long long id, const std::vector<float>& vertices)

            // for (auto& plData : m_vPolylineDatas)
            // {
            //     for (auto& id : plData.vIds)
            //     {
            //         if (id % 4 == 0)
            //         {
            //             m_plVboManager.updatePolyline(id, plData.vVerts);
            //         }
            //     }
            // }
        }

        // 调用新创建的渲染窗口函数
        //showPolylineRenderWindow(&m_plVboManager);
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
        vTriDatas[0].vVerts = { -0.4f, 0.4f, -0.5f, 0.2f, 0.4f, -0.5f, -0.4f, -0.4f, -0.5f };
        vTriDatas[0].brush = { 1.0f, 0.0f, 0.0f, dAlpha, dDepth };

        // 绿色，右上
        dAlpha = FakeDataBase::getRandomFloat(0.2f, 1.0f);
        dDepth = FakeDataBase::getRandomFloat(-1.0f, 1.0f);
        vTriDatas[1].vVerts = { -0.2f, 0.4f, -0.6f, 0.4f, 0.4f, -0.6f, 0.4f, -0.4f, -0.6f };
        vTriDatas[1].vIndices = { 0, 1, 2 };
        vTriDatas[1].brush = { 0.0f, 1.0f, 0.0f, dAlpha, dDepth };

        // 蓝色，左下
        dAlpha = FakeDataBase::getRandomFloat(0.2f, 1.0f);
        dDepth = FakeDataBase::getRandomFloat(-1.0f, 1.0f);
        vTriDatas[2].vVerts = { -0.4f, 0.0f, -0.7f, 0.2f, 0.0f, -0.7f, -0.4f, -0.6f, -0.7f };
        vTriDatas[2].vIndices = { 0, 1, 2 };
        vTriDatas[2].brush = { 0.0f, 0.0f, 1.0f, dAlpha, dDepth };

        // 黄色，右下
        dAlpha = FakeDataBase::getRandomFloat(0.2f, 1.0f);
        dDepth = FakeDataBase::getRandomFloat(-1.0f, 1.0f);
        vTriDatas[3].vVerts = { -0.2f, 0.0f, -0.8f, 0.4f, 0.0f, -0.8f, 0.4f, -0.6f, -0.8f };
        vTriDatas[3].vIndices = { 0, 1, 2 };
        vTriDatas[3].brush = { 1.0f, 1.0f, 0.0f, dAlpha, dDepth };
        return vTriDatas;
    }

    std::vector<TriangleData> FakeDataProvider::genRandomTriangleData(size_t vCount /*=10*/)
    {
        std::vector<TriangleData> vTriDatas;
        vTriDatas.reserve(vCount);

        for (size_t i = 0; i < vCount; ++i)
        {
            FakeTriangleData fakeTriangleData;
            fakeTriangleData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
            fakeTriangleData.generateTriangles(10);

            TriangleData triData{};
            triData.vVerts = fakeTriangleData.getVertices();
            triData.vIndices = fakeTriangleData.getIndices();

            Color c = fakeTriangleData.genRandomColor();
            // float a = fakeTriangleData.getRandomFloat(0.3f, 1.0f);
            float d = fakeTriangleData.getRandomFloat(-1.0f, 1.0f);
            triData.brush = { c, d };

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
        qWarning() << "FakeDataProvider: genFileTextureData OK";

        return vTexDatas;
    }

    std::vector<TextureData> FakeDataProvider::genRandomTextureData(size_t vCount /*=10*/)
    {
        std::vector<TextureData> vTexDatas;

        return vTexDatas;
    }

    std::vector<InstanceTexData> FakeDataProvider::genInstanceTextureData(GLuint& tex, int& vCount, size_t instanceCount /*=20*/)
    {
        std::vector<InstanceTexData> vInstances;
        return vInstances;
    }

    void FakeDataProvider::cleanup()
    {
    }
}