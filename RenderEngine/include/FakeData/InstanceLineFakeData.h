#ifndef INSTANCE_LINE_FAKE_DATA_H
#define INSTANCE_LINE_FAKE_DATA_H

#include <vector>
#include "FakeDataGenerator.h"
#include "Render/InstanceLineRenderer.h"
#include "Common/Color.h"

namespace GLRhi
{
    /**
     * @brief 实例化线段伪数据生成类
     */
    class InstanceLineFakeData final : public FakeDataGenerator
    {
    public:
        InstanceLineFakeData();
        ~InstanceLineFakeData() override;

    public:
        /**
         * @brief 生成指定数量的实例化线段
         * @param lineCount 要生成的线段数量
         * @param minWidth 线段最小宽度
         * @param maxWidth 线段最大宽度
         */
        void genLines(int lineCount = 100, float minWidth = 0.001f, float maxWidth = 0.003f);

        /**
         * @brief 获取生成的实例化线段数据
         * @return 实例化线段数据数组
         */
        const std::vector<InstanceLineData>& getInstanceData() const;

        /**
         * @brief 清空数据
         */
        void clear() override;

    private:
        /**
         * @brief 生成单个实例化线段
         * @param minWidth 线段最小宽度
         * @param maxWidth 线段最大宽度
         * @return 实例化线段数据
         */
        InstanceLineData genSingleLine(float minWidth, float maxWidth);



    private:
        std::vector<InstanceLineData> m_instanceData; // 实例化线段数据
    };
}

#endif // INSTANCE_LINE_FAKE_DATA_H