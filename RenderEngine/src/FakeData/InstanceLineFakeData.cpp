#include "FakeData/InstanceLineFakeData.h"

namespace GLRhi
{
    InstanceLineFakeData::InstanceLineFakeData()
    {
    }

    InstanceLineFakeData::~InstanceLineFakeData()
    {
        clear();
    }

    void InstanceLineFakeData::genLines(int lineCount, float minWidth, float maxWidth)
    {
        clear();
        m_instanceData.reserve(lineCount);

        for (int i = 0; i < lineCount; ++i)
        {
            InstanceLineData lineData = genSingleLine(minWidth, maxWidth);
            m_instanceData.push_back(lineData);
        }
    }

    std::vector<InstanceLineData>& InstanceLineFakeData::getInstanceData()
    {
        return m_instanceData;
    }

    void InstanceLineFakeData::clear()
    {
        m_instanceData.clear();
    }

    InstanceLineData InstanceLineFakeData::genSingleLine(float minWidth, float maxWidth)
    {
        InstanceLineData lineData;

        // 生成线段的起点和终点
        float startX = getRandomFloat(m_xMin, m_xMax);
        float startY = getRandomFloat(m_yMin, m_yMax);
        float endX = getRandomFloat(m_xMin, m_xMax);
        float endY = getRandomFloat(m_yMin, m_yMax);

        // 设置起点和终点
        lineData.pos1[0] = startX;
        lineData.pos1[1] = startY;
        lineData.pos1[2] = 0.0f;

        lineData.pos2[0] = endX;
        lineData.pos2[1] = endY;
        lineData.pos2[2] = 0.0f;

        // 生成随机颜色
        Color color = genRandomColor();
        lineData.color[0] = color.getRed();
        lineData.color[1] = color.getGreen();
        lineData.color[2] = color.getBlue();
        lineData.color[3] = color.getAlpha();

        // 生成随机宽度
        lineData.width = getRandomFloat(minWidth, maxWidth);

        // 生成随机深度值（用于绘制顺序）
        lineData.depth = getRandomFloat(0.0f, 1.0f);

        return lineData;
    }
}