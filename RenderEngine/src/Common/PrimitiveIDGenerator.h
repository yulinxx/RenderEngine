#ifndef PRIMITIVE_ID_GENERATOR_H
#define PRIMITIVE_ID_GENERATOR_H

#include <atomic>
#include <limits>
#include <stdexcept>

namespace GLRhi
{
    /**
     * @brief 唯一的图元ID生成器
     * 线程安全的ID生成器，用于为渲染图元分配唯一标识符
     * 有超过 9.22 亿亿 的唯一 ID,现实中几乎用不完 
     * 所有实例共享同一个ID序列，确保全局唯一性
     */
    class PrimitiveIDGenerator
    {
    private:
        // 静态成员变量，所有实例共享同一个计数器
        static std::atomic<long long> s_nNextID;

    public:
        /**
         * @brief 生成下一个唯一的图元ID
         * @return 唯一的图元ID
         * @note 当ID达到最大值时，会从负值最小值开始继续分配
         * @note 所有实例共享同一个ID序列，确保全局唯一性
         */
        long long genID()
        {
            long long nNewID = s_nNextID.fetch_add(1);
            
            // 检查是否达到最大值，如果达到则重置为最小值
            if (nNewID == std::numeric_limits<long long>::max())
            {
                // 从long long的最小值开始（负值）
                s_nNextID.store(std::numeric_limits<long long>::min());
            }

            return nNewID;
        }

        /**
         * @brief 重置计数器
         * @note 仅用于测试
         */
        void reset()
        {
            s_nNextID.store(1);
        }

        /**
         * @brief 获取当前最大的 ID 值
         * @return 当前已分配的最大ID
         * @note 仅用于测试
         */
        long long getCurrentMaxID() const
        {
            return s_nNextID.load() - 1;
        }
    };
    
    // 静态成员变量初始化
    inline std::atomic<long long> PrimitiveIDGenerator::s_nNextID{1};
}

#endif // PRIMITIVE_ID_GENERATOR_H