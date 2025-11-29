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

        s_colorPool.reserve(7);

        s_colorPool.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
        s_colorPool.emplace_back(0.0f, 1.0f, 0.0f, 1.0f);
        s_colorPool.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);
        s_colorPool.emplace_back(1.0f, 1.0f, 0.0f, 1.0f);
        s_colorPool.emplace_back(1.0f, 0.0f, 1.0f, 1.0f);
        s_colorPool.emplace_back(0.0f, 1.0f, 1.0f, 1.0f);
        //s_colorPool.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
    }

    Color FakeDataBase::genRandomColor()
    {
        initializeColorPool();
        //return s_colorPool[0];

        int nIndex = getRandomInt(0, static_cast<int>(s_colorPool.size()) - 1);
        return s_colorPool[nIndex];
    }
}