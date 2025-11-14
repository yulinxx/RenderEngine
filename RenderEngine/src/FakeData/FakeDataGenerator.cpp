#include "FakeData/FakeDataGenerator.h"

// 初始化static成员变量
std::mt19937 GLRhi::FakeDataGenerator::m_generator;

namespace GLRhi
{
    FakeDataGenerator::FakeDataGenerator()
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

    void FakeDataGenerator::setRange(float xMin, float xMax, float yMin, float yMax)
    {
        m_xMin = xMin;
        m_xMax = xMax;
        m_yMin = yMin;
        m_yMax = yMax;
    }

    float FakeDataGenerator::getRandomFloat(float min, float max)
    {
        if (min >= max)
        {
            return min;
        }

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

    int FakeDataGenerator::getRandomInt(int min, int max)
    {
        if (min >= max)
        {
            return min;
        }

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

    Color FakeDataGenerator::genRandomColor()
    {
        float r = getRandomFloat(0.0f, 1.0f);
        float g = getRandomFloat(0.0f, 1.0f);
        float b = getRandomFloat(0.0f, 1.0f);
        float a = getRandomFloat(0.8f, 1.0f); // Alpha值稍微高一些
        return Color(r, g, b, a);
    }
}