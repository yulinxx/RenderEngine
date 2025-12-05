#include "FakeData/FakePolyLineData.h"
#include <cmath>
#include <QDebug>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GLRhi
{
    FakePolyLineData::FakePolyLineData()
    {
        // setRange(-1000.0f, 1000.0f, -1000.0f, 1000.0f);
    }

    FakePolyLineData::~FakePolyLineData()
    {
        clear();
    }

    void FakePolyLineData::generateLines(size_t lineCount, size_t minPoints, size_t maxPoints)
    {
        if (minPoints < 2)
            minPoints = 2;
        if (maxPoints > 5000)
            maxPoints = 5000;
        if (minPoints > maxPoints)
            minPoints = maxPoints;

        clear();

        // 生成指定数量的线段
        for (size_t i = 0; i < lineCount; ++i)
        {
            int pointCount = getRandomInt(static_cast<int>(minPoints), static_cast<int>(maxPoints));
            generateSingleLine(pointCount);

            //static size_t N = 3;
            //generateSingleLine(N);
            //N++;
        }
    }

    const std::vector<float>& FakePolyLineData::getVertices() const
    {
        return m_vertices;
    }

    const std::vector<size_t>& FakePolyLineData::getLineInfos() const
    {
        return m_lineInfos;
    }

    void FakePolyLineData::clear()
    {
        m_vertices.clear();
        m_lineInfos.clear();
    }

    void FakePolyLineData::generateSingleLine(int nPointSz)
    {
        if (nPointSz < 2)
            nPointSz = 2;

        // 生成线段的起始点
        float startX = getRandomFloat(m_xMin, m_xMax);
        float startY = getRandomFloat(m_yMin, m_yMax);

        // 计算中心点作为圆的中心
        float centerX = startX;
        float centerY = startY;

        // 设置最大半径（使用范围的一半）
        float maxRadius = std::min((m_xMax - m_xMin) * 0.4f, (m_yMax - m_yMin) * 0.4f);

        // 生成起始点（在圆内）
        float radius = getRandomFloat(0.0f, maxRadius * 0.8f); // 起始点不要太靠近边缘
        float angle = getRandomFloat(0.0f, static_cast<float>(2.0f * M_PI));
        float startZ = 0.0f;

        auto clipXYZ = [this](float& x, float& y) {
            x = std::clamp(x, -0.98f, 0.98f);
            y = std::clamp(y, -0.98f, 0.98f);
            };

        // 添加起始点到顶点数据
        m_vertices.push_back(startX);
        m_vertices.push_back(startY);
        m_vertices.push_back(startZ);

        clipXYZ(startX, startY);

        //qDebug() << "\n--- startX:" << startX << ", startY:" << startY;

        // 生成线段的其余点
        for (int i = 1; i < nPointSz; ++i)
        {
            // 获取前一个点
            float prevX = m_vertices[m_vertices.size() - 3];
            float prevY = m_vertices[m_vertices.size() - 2];

            // 计算前一个点到中心的距离
            float distToCenter = std::sqrt((prevX - centerX) * (prevX - centerX) +
                (prevY - centerY) * (prevY - centerY));

            // 计算前一个点的方向角度
            float prevAngle = std::atan2(prevY - centerY, prevX - centerX);

            // 根据距离调整移动方向：如果靠近边缘，则向中心方向偏移更多
            float angleOffset = getRandomFloat(static_cast<float>(-M_PI / 6.0f), static_cast<float>(M_PI / 6.0f)); // 最多30度的方向变化
            float moveDistance = getRandomFloat(maxRadius * 0.02f, maxRadius * 0.1f); // 控制每步移动距离

            // 如果接近边缘，调整角度使其更倾向于向中心移动
            if (distToCenter > maxRadius * 0.8f)
            {
                angleOffset -= (prevAngle - std::atan2(-prevY + centerY, -prevX + centerX));
                angleOffset *= 0.5f; // 减轻强制向中心的效果
            }

            // 计算新角度和新距离
            float newAngle = prevAngle + angleOffset;
            float newDist = distToCenter + moveDistance * (getRandomFloat(-0.5f, 1.5f) - 0.5f);

            // 确保距离在有效范围内
            newDist = std::max(0.05f, std::min(maxRadius, newDist));

            // 计算新点坐标
            float nextX = centerX + newDist * std::cos(newAngle);
            float nextY = centerY + newDist * std::sin(newAngle);
            float nextZ = 0.0f;

            // 添加下一个点到顶点数据
            m_vertices.push_back(nextX);
            m_vertices.push_back(nextY);
            m_vertices.push_back(nextZ);

            clipXYZ(nextX, nextY);

            //qDebug() << "nextX:" << nextX << ", nextY:" << nextY;
        }

        // 记录当前线段的信息
        m_lineInfos.push_back(nPointSz);
    }
}