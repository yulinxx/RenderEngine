#include "FakeData/FakeDataBase.h"

// Define static members
std::mt19937 g_generator;
std::vector<GLRhi::Color> g_colorPool;

FakeDataBase::FakeDataBase()
    : m_impl(new FakeDataBaseImpl())
{
    try
    {
        getGenerator() = std::mt19937(m_impl->m_randomDevice());
    }
    catch (...)
    {
        getGenerator() = std::mt19937(42);
    }
}

std::mt19937& FakeDataBase::getGenerator()
{
    return g_generator;
}

std::vector<GLRhi::Color>& FakeDataBase::getColorPool()
{
    return g_colorPool;
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
        return distribution(getGenerator());
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
        return distribution(getGenerator());
    }
    catch (...)
    {
        return min;
    }
}

void FakeDataBase::initializeColorPool()
{
    auto& colorPool = getColorPool();
    if (!colorPool.empty())
        return;

    colorPool.reserve(20);

    // 基本颜色
    colorPool.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);     // 红色
    colorPool.emplace_back(0.0f, 1.0f, 0.0f, 1.0f);     // 绿色
    colorPool.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);     // 蓝色
    colorPool.emplace_back(1.0f, 1.0f, 0.0f, 1.0f);     // 黄色
    colorPool.emplace_back(1.0f, 0.0f, 1.0f, 1.0f);     // 洋红色
    colorPool.emplace_back(0.0f, 1.0f, 1.0f, 1.0f);     // 青色
    colorPool.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);    // 白色

    // 灰色系列
    colorPool.emplace_back(0.0f, 0.0f, 0.0f, 1.0f);     // 黑色
    colorPool.emplace_back(0.5f, 0.5f, 0.5f, 1.0f);     // 灰色
    colorPool.emplace_back(0.25f, 0.25f, 0.25f, 1.0f);  // 深灰色
    colorPool.emplace_back(0.75f, 0.75f, 0.75f, 1.0f);  // 浅灰色

    // 其他常用颜色
    colorPool.emplace_back(1.0f, 0.65f, 0.0f, 1.0f);    // 橙色
    colorPool.emplace_back(0.5f, 0.0f, 0.5f, 1.0f);     // 紫色
    colorPool.emplace_back(0.65f, 0.16f, 0.16f, 1.0f);  // 棕色
    colorPool.emplace_back(1.0f, 0.75f, 0.8f, 1.0f);    // 粉色
    colorPool.emplace_back(0.75f, 1.0f, 0.0f, 1.0f);    // 酸橙绿
    colorPool.emplace_back(0.0f, 0.5f, 0.5f, 1.0f);     // 蓝绿色
    colorPool.emplace_back(0.0f, 0.0f, 0.5f, 1.0f);     // 藏青色
    colorPool.emplace_back(0.5f, 0.0f, 0.0f, 1.0f);     // 栗色
    colorPool.emplace_back(0.5f, 0.5f, 0.0f, 1.0f);     // 橄榄绿
}

GLRhi::Color FakeDataBase::genRandomColor()
{
    initializeColorPool();
    //return getColorPool()[0];

    int nIndex = getRandomInt(0, static_cast<int>(getColorPool().size()) - 1);
    return getColorPool()[nIndex];
}