#include "FakeData/FakeDataBase.h"

std::mt19937 GLRhi::FakeDataBase::m_generator;
std::vector<GLRhi::Color> GLRhi::FakeDataBase::s_colorPool;

namespace GLRhi
{
    FakeDataBase::FakeDataBase()
    {
        try
        {
            m_generator = std::mt19937(m_randomDevice());
        }
        catch (...)
        {
            m_generator = std::mt19937(42);
        }
    }

    void FakeDataBase::setRange(float xMin, float xMax, float yMin, float yMax)
    {
        m_xMin = xMin;
        m_xMax = xMax;
        m_yMin = yMin;
        m_yMax = yMax;
    }

    float FakeDataBase::getRandomFloat(float min, float max)
    {
        if (min >= max)
            return min;

        try
        {
            std::uniform_real_distribution<float> distribution(min, max);
            return distribution(m_generator);
        }
        catch (...)
        {
            return min;
        }
    }

    int FakeDataBase::getRandomInt(int min, int max)
    {
        if (min >= max)
            return min;

        try
        {
            std::uniform_int_distribution<int> distribution(min, max);
            return distribution(m_generator);
        }
        catch (...)
        {
            return min;
        }
    }

    void FakeDataBase::initializeColorPool()
    {
        if (!s_colorPool.empty())
            return;

        s_colorPool.reserve(20);

        // 基本颜色
        s_colorPool.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);     // 红色
        s_colorPool.emplace_back(0.0f, 1.0f, 0.0f, 1.0f);     // 绿色
        s_colorPool.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);     // 蓝色
        s_colorPool.emplace_back(1.0f, 1.0f, 0.0f, 1.0f);     // 黄色
        s_colorPool.emplace_back(1.0f, 0.0f, 1.0f, 1.0f);     // 洋红色
        s_colorPool.emplace_back(0.0f, 1.0f, 1.0f, 1.0f);     // 青色
        s_colorPool.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);    // 白色
        
        // 灰色系列
        s_colorPool.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);     // 黑色
        s_colorPool.emplace_back(0.5f, 0.5f, 0.5f, 1.0f);     // 灰色
        s_colorPool.emplace_back(0.25f, 0.25f, 0.25f, 1.0f);  // 深灰色
        s_colorPool.emplace_back(0.75f, 0.75f, 0.75f, 1.0f);  // 浅灰色
        
        // 其他常用颜色
        s_colorPool.emplace_back(1.0f, 0.65f, 0.0f, 1.0f);    // 橙色
        s_colorPool.emplace_back(0.5f, 0.0f, 0.5f, 1.0f);     // 紫色
        s_colorPool.emplace_back(0.65f, 0.16f, 0.16f, 1.0f);  // 棕色
        s_colorPool.emplace_back(1.0f, 0.75f, 0.8f, 1.0f);    // 粉色
        s_colorPool.emplace_back(0.75f, 1.0f, 0.0f, 1.0f);    // 酸橙绿
        s_colorPool.emplace_back(0.0f, 0.5f, 0.5f, 1.0f);     // 蓝绿色
        s_colorPool.emplace_back(0.0f, 0.0f, 0.5f, 1.0f);     // 藏青色
        s_colorPool.emplace_back(0.5f, 0.0f, 0.0f, 1.0f);     // 栗色
        s_colorPool.emplace_back(0.5f, 0.5f, 0.0f, 1.0f);     // 橄榄绿
        
    }

    Color FakeDataBase::genRandomColor()
    {
        initializeColorPool();
        //return s_colorPool[0];

        int nIndex = getRandomInt(0, static_cast<int>(s_colorPool.size()) - 1);
        return s_colorPool[nIndex];
    }
}