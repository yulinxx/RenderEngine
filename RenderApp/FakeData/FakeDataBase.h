#ifndef FAKE_DATA_BASE_H
#define FAKE_DATA_BASE_H

#include <random>
#include <vector>
#include "Common/Color.h"

struct FakeDataBaseImpl
{
    std::random_device m_randomDevice;
};

class FakeDataBase
{
public:
    FakeDataBase();
    // 内联实现析构函数以避免重复定义并确保正确释放资源
    virtual ~FakeDataBase()
    {
        if (m_impl)
            delete m_impl;
    }

public:
    void setRange(float xMin, float xMax, float yMin, float yMax);

    static float getRandomFloat(float min, float max);
    static int getRandomInt(int min, int max);
    virtual void clear() = 0;

    static GLRhi::Color genRandomColor();

protected:
    float m_xMin = -1.0f;
    float m_xMax = 1.0f;
    float m_yMin = -1.0f;
    float m_yMax = 1.0f;

    // Use static member functions to access static data instead of exposing STL containers directly
    static std::mt19937& getGenerator();
    static std::vector<GLRhi::Color>& getColorPool();

    // Use PIMPL pattern for non-static STL member
    FakeDataBaseImpl* m_impl = nullptr;

    static void initializeColorPool();
};

#endif // FAKE_DATA_BASE_H