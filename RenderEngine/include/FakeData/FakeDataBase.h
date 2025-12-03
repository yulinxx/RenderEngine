#ifndef FAKE_DATA_BASE_H
#define FAKE_DATA_BASE_H

#include <random>
#include <vector>
#include "Common/Color.h"
#include "Common/DllSet.h"

namespace GLRhi
{
    // Forward declaration of PIMPL struct
    struct FakeDataBaseImpl;

    class GLRENDER_API FakeDataBase
    {
    public:
        FakeDataBase();
        // 内联实现析构函数以避免重复定义并确保正确释放资源
        virtual ~FakeDataBase() { delete m_impl; }

    public:
        void setRange(float xMin, float xMax, float yMin, float yMax);

        static float getRandomFloat(float min, float max);
        static int getRandomInt(int min, int max);
        virtual void clear() = 0;

        static Color genRandomColor();

    protected:
        float m_xMin = -1.0f;
        float m_xMax = 1.0f;
        float m_yMin = -1.0f;
        float m_yMax = 1.0f;

        // Use static member functions to access static data instead of exposing STL containers directly
        static std::mt19937& getGenerator();
        static std::vector<Color>& getColorPool();
        
        // Use PIMPL pattern for non-static STL member
        FakeDataBaseImpl* m_impl;

        static void initializeColorPool();
    };
}

#endif // FAKE_DATA_BASE_H