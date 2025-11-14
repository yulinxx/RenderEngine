#ifndef FAKE_DATA_GENERATOR_H
#define FAKE_DATA_GENERATOR_H

#include <random>
#include <vector>
#include "Common/Color.h"

namespace GLRhi
{
    /**
     * @brief 测试伪数据生成器基类
     */
    class FakeDataGenerator
    {
    public:
        FakeDataGenerator();
        virtual ~FakeDataGenerator() = default;

        /**
         * @brief 设置XY坐标范围
         * @param xMin X轴最小值
         * @param xMax X轴最大值
         * @param yMin Y轴最小值
         * @param yMax Y轴最大值
         */
        void setRange(float xMin, float xMax, float yMin, float yMax);

        /**
         * @brief 生成随机浮点数
         * @param min 最小值
         * @param max 最大值
         * @return 随机浮点数
         */
        static float getRandomFloat(float min, float max);

        /**
         * @brief 生成随机整数
         * @param min 最小值
         * @param max 最大值
         * @return 随机整数
         */
        static int getRandomInt(int min, int max);

        /**
         * @brief 清空数据
         */
        virtual void clear() = 0;

        /**
         * @brief 生成随机颜色
         * @return 随机颜色值
         */
        static Color genRandomColor();

    protected:
        // 坐标范围
        float m_xMin = -1.0f;
        float m_xMax = 1.0f;
        float m_yMin = -1.0f;
        float m_yMax = 1.0f;

        static std::mt19937 m_generator;
        std::random_device m_randomDevice;
    };
}

#endif // FAKE_DATA_GENERATOR_H