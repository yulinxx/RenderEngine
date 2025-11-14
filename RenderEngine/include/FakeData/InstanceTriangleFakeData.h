#ifndef INSTANCE_TRIANGLE_FAKE_DATA_H
#define INSTANCE_TRIANGLE_FAKE_DATA_H

#include <vector>
#include "FakeDataGenerator.h"
#include "Render/InstanceTriangleRenderer.h"
#include "Common/Color.h"

namespace GLRhi
{
    /**
     * @brief 实例化三角形伪数据生成类
     */
    class InstanceTriangleFakeData final : public FakeDataGenerator
    {
    public:
        InstanceTriangleFakeData();
        ~InstanceTriangleFakeData() override;

    public:
        /**
         * @brief 生成指定数量的实例化三角形
         * @param triangleCount 要生成的三角形数量
         * @param minSize 三角形最小尺寸
         * @param maxSize 三角形最大尺寸
         */
        void genTriangles(int triangleCount = 100, float minSize = 0.01f, float maxSize = 0.1f);

        /**
         * @brief 获取生成的实例化三角形数据
         * @return 实例化三角形数据数组
         */
        const std::vector<InstanceTriangleData>& getInstanceData() const;

        /**
         * @brief 清空数据
         */
        void clear() override;

    private:
        /**
         * @brief 生成单个实例化三角形
         * @param minSize 三角形最小尺寸
         * @param maxSize 三角形最大尺寸
         * @return 实例化三角形数据
         */
        InstanceTriangleData genSingleTriangle(float minSize, float maxSize);



    private:
        std::vector<InstanceTriangleData> m_instanceData; // 实例化三角形数据
    };
}

#endif // INSTANCE_TRIANGLE_FAKE_DATA_H