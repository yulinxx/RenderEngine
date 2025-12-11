#ifndef TRIANGLE_VBO_MANAGER_H
#define TRIANGLE_VBO_MANAGER_H

#include "Common/DllSet.h"
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>
#include <thread>
#include <map>
#include "Render/RenderCommon.h"
#include <QOpenGLFunctions_3_3_Core>

namespace GLRhi
{
    /**
     * @brief 三角形图元信息结构体
     *
     * 存储单个多边形（三角剖分后）的基本信息，包括标识符、索引数量、基础顶点偏移和有效性状态。
     * 这些信息用于渲染和管理三角形数据。
     */
    struct TrianglePrimitiveInfo
    {
        long long id{ -1 };          // 图元唯一标识符（多边形ID）
        GLsizei   nIndexCount{ 0 };  // 索引数量（三角形数量 * 3）
        GLint     nBaseVertex{ 0 };  // 基础顶点偏移量，用于索引复用
        size_t    nBaseIndex{ 0 };   // 索引在EBO中的起始位置（用于渲染时计算偏移）
        bool      bValid{ true };    // 图元有效性标志（false表示已删除）
    };

    /**
     * @brief 颜色VBO块结构体
     *
     * 表示一组具有相同颜色的三角形数据块，包含OpenGL缓冲区对象、容量信息、渲染命令和图元数据。
     * 每个块管理同一种颜色的三角形，实现颜色批处理渲染优化。
     */
    struct TriangleColorVBOBlock
    {
        unsigned int vao{ 0 };          // 顶点数组对象
        unsigned int vbo{ 0 };          // 顶点缓冲区对象
        unsigned int ebo{ 0 };          // 索引缓冲区对象
        Color color;                    // 该块所有三角形的统一颜色

        size_t nVertexCapacity{ 0 };    // 顶点容量上限
        size_t nIndexCapacity{ 0 };     // 索引容量上限
        size_t nVertexCount{ 0 };       // 当前实际使用的顶点数
        size_t nIndexCount{ 0 };        // 当前实际使用的索引数

        std::vector<GLsizei> vDrawCounts;       // 每个图元的索引数量数组，用于批量绘制
        std::vector<GLint>   vBaseVertices;     // 每个图元的基础顶点偏移数组
        std::vector<TrianglePrimitiveInfo> vPrimitives; // 图元信息数组

        std::unordered_map<long long, size_t> idToIndexMap; // 图元ID到索引的映射，用于快速查找

        bool bDirty{ false };           // 标记绘制命令是否需要重建
        bool bCompact{ false };         // 标记是否需要进行内存碎片整理
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @class TriangleVboManager
     * @brief 高性能三角形渲染管理器
     *
     * 该类是一个高度优化的OpenGL三角形渲染管理系统，专为处理大量动态三角形数据而设计。
     * 它提供了高效的内存管理、批量渲染和动态更新能力。
     *
     * 主要特性：
     * - 按颜色分组管理三角形数据，最小化状态切换
     * - 支持动态添加、删除、更新三角形，无需重建整个缓冲区
     * - 自动内存碎片整理，保持长期稳定的内存使用效率
     * - 后台线程进行资源优化，不阻塞渲染线程
     * - 增量数据上传策略，减少GPU通信开销
     * - 使用VAO/VBO/EBO进行高效渲染，支持OpenGL 3.3+
     * - 支持多边形三角剖分后的批量三角形管理
     */
    class GLRENDER_API TriangleVboManager final
    {
    public:
        TriangleVboManager();
        ~TriangleVboManager();

    public:
        /**
         * @brief 添加一个多边形（三角剖分后的三角形）
         * 将一个多边形的三角剖分结果添加到管理器中，自动按颜色分组存储。
         * @param id 多边形唯一标识符
         * @param vertices 顶点数据，格式为[x1,y1,z1,x2,y2,z2,...]，每个顶点3个浮点数
         * @param vertexCount 顶点数量（vertices数组长度 / 3）
         * @param indices 索引数据，格式为[i1,i2,i3,i4,i5,i6,...]，每3个索引组成一个三角形
         * @param indexCount 索引数量（indices数组长度）
         * @param color 三角形颜色
         * @return true成功添加，false失败（无效参数或ID已存在）
         * @note 索引数量必须是3的倍数（每个三角形3个索引）
         */
        bool addTriangle(long long id, float* vertices, size_t vertexCount,
            unsigned int* indices, size_t indexCount, const Color& color);

        /**
         * @brief 批量添加多个多边形（三角剖分后的三角形）
         * 一次性添加多个多边形，比单个添加更高效。
         * @param vTriangleDatas 三角形数据向量，每个元素为 (id, vertices, vertexCount, indices, indexCount, color)
         * @return 成功添加的多边形数量
         */
        size_t addTriangles(const std::vector<std::tuple<long long, float*, size_t, unsigned int*, size_t, Color>>& vTriangleDatas);

        /**
         * @brief 删除指定ID的多边形
         * 从管理器中移除指定ID的多边形，不立即释放内存而是标记为待清理。
         * @param id 要删除的多边形ID
         * @return true删除成功，false未找到该ID的多边形
         */
        bool removeTriangle(long long id);

        /**
         * @brief 批量删除多边形
         * @param vIds 要删除的多边形ID列表
         * @return 实际删除的多边形数量
         */
        size_t removeTriangles(const std::vector<long long>& vIds);

        /**
         * @brief 更新多边形数据
         * 更新指定ID多边形的顶点和索引数据，支持顶点和索引数量变化。
         * @param id 要更新的多边形ID
         * @param vertices 新的顶点数据
         * @param vertexCount 新的顶点数量
         * @param indices 新的索引数据
         * @param indexCount 新的索引数量
         * @return true更新成功，false未找到该ID或参数无效
         */
        bool updateTriangle(long long id, float* vertices, size_t vertexCount,
            unsigned int* indices, size_t indexCount);

        /**
         * @brief 设置多边形可见性
         * 控制指定ID多边形的可见性状态。
         * @param id 要设置的多边形ID
         * @param visible 是否可见
         * @return true设置成功，false未找到该ID的多边形
         */
        bool setTriangleVisible(long long id, bool visible);

        /**
         * @brief 清空所有多边形
         * 移除并释放所有多边形数据和相关资源。
         */
        void clearAllPrimitives();

        /**
         * @brief 渲染所有可见的多边形
         * 按颜色分组批量渲染所有可见的多边形，是系统的核心渲染方法。
         * 应在OpenGL渲染上下文中调用。
         */
        void renderVisiblePrimitives(); // glDrawElementsBaseVertex
        void renderVisiblePrimitivesEx(); // glMultiDrawElementsBaseVertex

        /**
         * @brief 启动后台碎片整理线程
         * 启动一个单独的线程进行内存碎片整理，定期检查并压缩需要整理的块。
         */
        void startBackgroundDefrag();

        /**
         * @brief 停止后台碎片整理线程
         */
        void stopBackgroundDefrag();

    private:
        /**
         * @brief 查找或创建指定颜色的VBO块
         *
         * 首先查找现有可用的同色块，不存在则创建新块。
         *
         * @param color 目标颜色
         * @return 指向TriangleColorVBOBlock的指针，失败返回nullptr
         */
        TriangleColorVBOBlock* getColorBlock(const Color& color);

        /**
         * @brief 创建新的颜色VBO块
         * 分配并初始化新的TriangleColorVBOBlock对象及其OpenGL资源。
         * @param color 块颜色
         * @return 指向新创建块的指针
         */
        TriangleColorVBOBlock* createNewColorBlock(const Color& color);

        /**
         * @brief 确保VBO块有足够容量
         * 检查并在必要时扩容指定的VBO块。
         * @param block 要检查的块
         * @param needVert 需要的顶点数量
         * @param needIdx 需要的索引数量
         */
        void checkBlockCapacity(TriangleColorVBOBlock* block, size_t needVert, size_t needIdx);

        /**
         * @brief 增量上传单个图元
         * 将单个图元的数据上传到GPU，只更新必要的部分。
         * @param block 目标块
         * @param primIdx 图元在块中的索引
         */
        void uploadSinglePrimitive(TriangleColorVBOBlock* block, size_t primIdx);

        /**
         * @brief 压缩内存块
         * 移除已删除的图元并重建内存布局，消除空洞。
         * @param block 要压缩的块
         */
        void compactBlock(TriangleColorVBOBlock* block);

        /**
         * @brief 重建绘制命令
         * 根据块中的图元数据重新生成批量绘制命令。
         * @param block 要重建命令的块
         */
        void rebuildDrawCmds(TriangleColorVBOBlock* block);

        void touchCache(long long id);

        /**
         * @brief 绑定块的OpenGL资源
         *
         * 激活指定块的VAO、VBO、EBO等资源。
         *
         * @param block 要绑定的块
         */
        void bindBlock(TriangleColorVBOBlock* block) const;

        /**
         * @brief 解绑当前块的OpenGL资源
         *
         * 安全地解除当前绑定的资源。
         */
        void unbindBlock() const;

    private:
        QOpenGLFunctions_3_3_Core* m_gl{ nullptr };
        mutable std::shared_mutex m_mutex;

        std::unordered_map<uint32_t, std::vector<TriangleColorVBOBlock*>> m_colorBlocksMap; // 按颜色键分组的VBO块映射
        /**
         * @brief 位置信息结构体
         * 存储多边形在系统中的精确位置，用于快速查找和更新。
         */
        struct Location
        {
            uint32_t colorKey{ 0 };             // 颜色哈希键值
            Color    color;                      // 实际颜色值
            TriangleColorVBOBlock* block{ nullptr };    // 所属VBO块
            size_t   nPrimIdx{ 0 };             // 在块中的图元索引
        };
        std::unordered_map<long long, Location> m_IDLocationMap; // ID到位置的快速映射

        // 顶点和索引缓存（用于增量上传和 compact）
        struct TriangleData
        {
            std::vector<float> vertices;
            std::vector<unsigned int> indices;
        };
        std::unordered_map<long long, TriangleData> m_vTriangleCache; // 原始顶点和索引数据缓存
        // LRU
        std::list<long long> m_triangleCacheOrder;
        static constexpr size_t MAX_CACHE_SIZE = 5000;  // 只缓存最近 5000 个被改过的多边形

        // 后台碎片整理相关
        std::thread m_defragThread;                 // 后台碎片整理线程
        std::atomic<bool> m_bStopDefrag{ false };   // 线程停止标志
    };
}

#endif // TRIANGLE_VBO_MANAGER_H

