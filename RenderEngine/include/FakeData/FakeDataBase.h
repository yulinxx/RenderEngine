#ifndef FAKE_DATA_BASE_H
#define FAKE_DATA_BASE_H

#include <random>
#include <vector>
#include "Common/Color.h"

namespace GLRhi
{
    class FakeDataBase
    {
    public:
        FakeDataBase();
        virtual ~FakeDataBase() = default;

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

        static std::mt19937 m_generator;
        static std::vector<Color> s_colorPool;
        std::random_device m_randomDevice;

        static void initializeColorPool();
    };
}

#endif // FAKE_DATA_BASE_H