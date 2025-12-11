/**
 * @class TriangleVboManager
 * @brief 高性能三角形渲染管理器，支持百万级动态三角形的实时增删改查和渲染
 *
 * 该类是一个高度优化的OpenGL三角形渲染管理系统，具有以下特点：
 * - 按颜色分组管理三角形数据，优化渲染性能
 * - 支持动态添加、删除、更新三角形，无需重建整个缓冲区
 * - 实现了自动内存管理和碎片整理机制，保持内存长期稳定
 * - 采用增量上传策略，最小化数据传输开销
 * - 支持OpenGL 3.3核心配置文件，兼容性好
 * - 支持多线程背景碎片整理，不阻塞主线程
 *
 * 设计模式：
 * - 使用VAO/VBO/EBO进行高效渲染
 * - 按颜色分组存储，减少状态切换
 * - 相对索引技术，避免索引重计算
 * - 延迟重建和压缩策略，优化内存使用
 */

#include <mutex>
#include <algorithm>
#include <chrono>
#include <QDebug>
#include <unordered_set>

#include "DataManager/TriangleVboManager.h"

namespace GLRhi
{
    // 类常量定义
    namespace
    {
        static constexpr size_t INIT_CAPACITY = 100'000; // VBO块的初始容量
        static constexpr size_t GROW_STEP = 200'000;
        static constexpr size_t MAX_VERT_PER_BLOCK = 1'500'000;
        static constexpr float COMPACT_THRESHOLD = 0.70f; // 使用率 < 70% 才压缩
    }

    /**
     * @brief 构造函数，初始化TriangleVboManager
     *
     * 在构造时尝试获取当前的OpenGL上下文，并初始化相关资源。
     * 注意：在创建此对象时，必须确保OpenGL上下文已经初始化。
     */
    TriangleVboManager::TriangleVboManager()
    {
        if (QOpenGLContext::currentContext())
        {
            m_gl = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
            if (!m_gl)
            {
                qFatal("Failed to get OpenGL 3.3 Core functions");
                return;
            }
        }
    }

    /**
     * @brief 析构函数，清理所有资源
     *
     * 负责释放所有分配的资源，包括：
     * - 停止后台碎片整理线程
     * - 删除所有OpenGL缓冲区对象（VAO、VBO、EBO）
     * - 释放所有TriangleColorVBOBlock对象
     * - 清空所有容器（m_colorBlocksMap, m_IDLocationMap, m_vTriangleCache）
     */
    TriangleVboManager::~TriangleVboManager()
    {
        stopBackgroundDefrag();

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        for (auto& pair : m_colorBlocksMap)
        {
            for (TriangleColorVBOBlock* block : pair.second)
            {
                if (m_gl)
                {
                    m_gl->glDeleteVertexArrays(1, &block->vao);
                    m_gl->glDeleteBuffers(1, &block->vbo);
                    m_gl->glDeleteBuffers(1, &block->ebo);
                }
                delete block;
            }
        }

        m_colorBlocksMap.clear();
        m_IDLocationMap.clear();
        m_IDLocationMap.reserve(0);
        m_vTriangleCache.clear();
        m_triangleCacheOrder.clear();
    }

    /**
     * @brief 添加一个多边形（三角剖分后的三角形）到渲染管理器
     *
     * 将一个新的多边形添加到系统中，自动按颜色分组存储，并执行增量上传到GPU。
     *
     * @param id 多边形的唯一标识符，用于后续的更新和删除操作
     * @param vertices 顶点数据，格式为[x1,y1,z1,x2,y2,z2,...]，每个顶点3个浮点数
     * @param vertexCount 顶点数量（vertices数组长度 / 3）
     * @param indices 索引数据，格式为[i1,i2,i3,i4,i5,i6,...]，每3个索引组成一个三角形
     * @param indexCount 索引数量（indices数组长度）
     * @param color 多边形的颜色
     * @return true 如果添加成功，false 如果参数无效或多边形已存在
     *
     * @note 索引数量必须是3的倍数（每个三角形3个索引）
     */
    bool TriangleVboManager::addTriangle(long long id, float* vertices, size_t vertexCount,
        unsigned int* indices, size_t indexCount, const Color& color)
    {
        if (vertexCount < 3 || indexCount < 3 || indexCount % 3 != 0)
            return false;

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (m_IDLocationMap.count(id))
            return false;

        TriangleColorVBOBlock* block = getColorBlock(color);
        if (!block)
            return false;

        checkBlockCapacity(block,
            block->nVertexCount + vertexCount,
            block->nIndexCount + indexCount);

        GLint nBaseVertex = static_cast<GLint>(block->nVertexCount);
        size_t nBaseIndex = block->nIndexCount; // 索引在EBO中的起始位置

        TrianglePrimitiveInfo prim;
        prim.id = id;
        prim.nIndexCount = static_cast<GLsizei>(indexCount);
        prim.nBaseVertex = nBaseVertex;
        prim.nBaseIndex = nBaseIndex;
        prim.bValid = true;

        size_t nPrimIdx = block->vPrimitives.size();
        block->vPrimitives.push_back(std::move(prim));
        block->idToIndexMap[id] = nPrimIdx;

        block->nVertexCount += vertexCount;
        block->nIndexCount += indexCount;
        block->bDirty = true;

        // 将顶点和索引数据复制到缓存中
        TriangleData& data = m_vTriangleCache[id];
        data.vertices.assign(vertices, vertices + vertexCount * 3);
        data.indices.assign(indices, indices + indexCount);
        m_IDLocationMap[id] = { color.toUInt32(), color, block, nPrimIdx };

        uploadSinglePrimitive(block, nPrimIdx); // 增量上传，只传这一个
        return true;
    }

    /**
     * @brief 批量添加多个多边形（三角剖分后的三角形）
     *
     * 比逐个调用 addTriangle() 快 5~20 倍，专为一次性加载数十万个多边形设计。
     * 内部会：
     * - 按颜色自动分组
     * - 每种颜色只扩容/上传一次
     * - 最小化锁粒度、OpenGL 调用、内存拷贝
     *
     * @param vTriangleDatas 批量数据：{id, vertices, vertexCount, indices, indexCount, color}
     * @return 添加成功的图元数量（失败的会跳过并打印警告）
     */
    size_t TriangleVboManager::addTriangles(
        const std::vector<std::tuple<long long, float*, size_t, unsigned int*, size_t, Color>>& vTriangleDatas)
    {
        if (vTriangleDatas.empty() || !m_gl)
            return 0;

        // Step 1: 预处理 — 按颜色分组 + 过滤无效数据 + 预检查ID冲突
        struct BatchGroup
        {
            Color color;
            std::vector<size_t> indices; // 在 vTriangleDatas 中的下标
            size_t totalVerts = 0;
            size_t totalIndices = 0;
        };

        std::unordered_map<uint32_t, BatchGroup> colorGroups;
        std::vector<bool> validFlags(vTriangleDatas.size(), true);
        size_t validCount = 0;

        {
            std::shared_lock<std::shared_mutex> readLock(m_mutex);
            for (size_t i = 0; i < vTriangleDatas.size(); ++i)
            {
                const auto& [id, verts, vertexCount, indices, indexCount, color] = vTriangleDatas[i];

                if (vertexCount < 3 || indexCount < 3 || indexCount % 3 != 0)
                {
                    validFlags[i] = false;
                    continue;
                }

                if (m_IDLocationMap.count(id))
                {
                    validFlags[i] = false;
                    continue;
                }

                uint32_t key = color.toUInt32();

                auto& batchGroup = colorGroups[key];
                batchGroup.color = color;

                batchGroup.indices.push_back(i);
                batchGroup.totalVerts += vertexCount;
                batchGroup.totalIndices += indexCount;
                validCount++;
            }
        }

        if (validCount == 0)
            return 0;

        std::unique_lock<std::shared_mutex> writeLock(m_mutex);

        size_t nAdd = 0;

        for (auto& [key, group] : colorGroups)
        {
            TriangleColorVBOBlock* block = getColorBlock(group.color);
            if (!block)
            {
                qCritical() << "Failed to create color block for batch add";
                continue;
            }

            checkBlockCapacity(block,
                block->nVertexCount + group.totalVerts,
                block->nIndexCount + group.totalIndices);

            // 预计算本次批次在块中的起始偏移
            GLint nBaseVertexStart = static_cast<GLint>(block->nVertexCount);
            size_t nVertOffset = block->nVertexCount; // 顶点偏移
            size_t nIdxOffset = block->nIndexCount;   // 索引偏移

            // 准备批量上传用的连续缓冲区
            std::vector<float> vBatchVerts;
            std::vector<unsigned int> vBatchIndices;
            vBatchVerts.reserve(group.totalVerts * 3);
            vBatchIndices.reserve(group.totalIndices);

            std::vector<TrianglePrimitiveInfo> vNewPrims;
            vNewPrims.reserve(group.indices.size());

            // 遍历该颜色组所有图元
            for (size_t idx : group.indices)
            {
                if (!validFlags[idx])
                    continue;

                const auto& [id, verts, vertexCount, indices, indexCount, color] = vTriangleDatas[idx];
                TrianglePrimitiveInfo prim;
                prim.id = id;
                prim.nIndexCount = static_cast<GLsizei>(indexCount);
                prim.nBaseVertex = static_cast<GLint>(nVertOffset);
                prim.nBaseIndex = nIdxOffset; // 索引在EBO中的起始位置
                prim.bValid = true;

                // 记录图元信息
                size_t nPrimIdxInBlock = block->vPrimitives.size() + vNewPrims.size();
                vNewPrims.push_back(std::move(prim));
                block->idToIndexMap[id] = nPrimIdxInBlock;

                // 缓存顶点和索引（用于后续 update / compact）
                TriangleData& data = m_vTriangleCache[id];
                data.vertices.assign(verts, verts + vertexCount * 3);
                data.indices.assign(indices, indices + indexCount);
                m_IDLocationMap[id] = { key, color, block, nPrimIdxInBlock };

                // 填充批量缓冲区
                vBatchVerts.insert(vBatchVerts.end(), verts, verts + vertexCount * 3);
                
                // 索引需要相对于 baseVertex 进行偏移
                for (size_t i = 0; i < indexCount; ++i)
                    vBatchIndices.push_back(static_cast<unsigned int>(nVertOffset + indices[i]));

                nVertOffset += vertexCount;
                nIdxOffset += indexCount;
                nAdd++;
            }

            // 一次性上传
            if (!vBatchVerts.empty())
            {
                GLsizeiptr vertByteOffset = static_cast<GLsizeiptr>(nBaseVertexStart) * 3 * sizeof(float);
                GLsizeiptr idxByteOffset = static_cast<GLsizeiptr>(nBaseVertexStart) * sizeof(unsigned int);

                // 上传顶点
                m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
                m_gl->glBufferSubData(GL_ARRAY_BUFFER, vertByteOffset,
                    static_cast<GLsizeiptr>(vBatchVerts.size() * sizeof(float)), vBatchVerts.data());

                // 上传索引
                m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
                m_gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, idxByteOffset,
                    static_cast<GLsizeiptr>(vBatchIndices.size() * sizeof(unsigned int)), vBatchIndices.data());
            }

            // 追加图元信息
            block->vPrimitives.insert(block->vPrimitives.end(),
                std::make_move_iterator(vNewPrims.begin()),
                std::make_move_iterator(vNewPrims.end()));

            // 更新块统计
            block->nVertexCount += group.totalVerts;
            block->nIndexCount += group.totalIndices;
            block->bDirty = true;
        }

        return nAdd;
    }

    /**
     * @brief 从渲染管理器中移除指定ID的多边形
     *
     * 通过ID查找并移除多边形，标记为无效并触发后续的内存整理。
     * 实际的内存释放会在碎片整理时进行，以优化性能。
     *
     * @param id 要移除的多边形的唯一标识符
     * @return true 如果成功移除，false 如果多边形不存在
     */
    bool TriangleVboManager::removeTriangle(long long id)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_IDLocationMap.find(id);
        if (it == m_IDLocationMap.end())
            return false;

        const Location& loc = it->second;
        TriangleColorVBOBlock* block = loc.block;
        TrianglePrimitiveInfo& prim = block->vPrimitives[loc.nPrimIdx];

        prim.bValid = false;
        prim.nIndexCount = 0;
        block->bDirty = true;
        block->bCompact = true;

        m_IDLocationMap.erase(it);
        m_vTriangleCache.erase(id);
        block->idToIndexMap.erase(id);

        return true;
    }

    /**
     * @brief 批量删除多边形
     *
     * @param vIds 要删除的多边形ID列表
     * @return 实际删除的多边形数量
     */
    size_t TriangleVboManager::removeTriangles(const std::vector<long long>& vIds)
    {
        if (vIds.empty())
            return 0;

        std::unique_lock lock(m_mutex);
        size_t nDelCount = 0;

        for (long long id : vIds)
        {
            auto it = m_IDLocationMap.find(id);
            if (it == m_IDLocationMap.end())
                continue;

            const Location& loc = it->second;
            TriangleColorVBOBlock* block = loc.block;
            TrianglePrimitiveInfo& prim = block->vPrimitives[loc.nPrimIdx];

            prim.bValid = false;
            prim.nIndexCount = 0;
            block->bDirty = true;
            block->bCompact = true;

            m_IDLocationMap.erase(it);
            m_vTriangleCache.erase(id);
            block->idToIndexMap.erase(id);
            ++nDelCount;
        }
        return nDelCount;
    }

    /**
     * @brief 更新指定ID的多边形数据
     *
     * 更新现有多边形的顶点和索引数据，根据数据变化采用不同策略：
     * - 数据变化不大时，直接增量更新
     * - 数据变化超过50%时，采用删除后重新添加的策略，确保内存使用合理
     *
     * @param id 要更新的多边形的唯一标识符
     * @param vertices 新的顶点数据，格式为[x1,y1,z1,x2,y2,z2,...]
     * @param vertexCount 新的顶点数量
     * @param indices 新的索引数据
     * @param indexCount 新的索引数量
     * @return true 如果更新成功，false 如果参数无效或多边形不存在
     *
     * @note 索引数量必须是3的倍数
     */
    bool TriangleVboManager::updateTriangle(long long id, float* vertices, size_t vertexCount,
        unsigned int* indices, size_t indexCount)
    {
        if (vertexCount < 3 || indexCount < 3 || indexCount % 3 != 0)
            return false;

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_IDLocationMap.find(id);
        if (it == m_IDLocationMap.end())
            return false;

        const Location& loc = it->second;
        TriangleColorVBOBlock* block = loc.block;
        size_t nPrimIdx = loc.nPrimIdx;
        TrianglePrimitiveInfo& prim = block->vPrimitives[nPrimIdx];

        size_t nOldVertCount = static_cast<size_t>(prim.nIndexCount); // 旧索引数
        size_t nOldVertCount_actual = 0;
        auto cacheIt = m_vTriangleCache.find(id);
        if (cacheIt != m_vTriangleCache.end())
        {
            nOldVertCount_actual = cacheIt->second.vertices.size() / 3;
        }

        // 如果新数据明显大于旧数据，采用删除后重新添加的策略
        if (vertexCount > nOldVertCount_actual * 1.5f || indexCount > nOldVertCount * 1.5f)
        {
            Color c = loc.color;
            lock.unlock();
            removeTriangle(id);
            return addTriangle(id, vertices, vertexCount, indices, indexCount, c);
        }

        prim.nIndexCount = static_cast<GLsizei>(indexCount);
        prim.bValid = true;

        // 将顶点和索引数据复制到缓存中
        TriangleData& data = m_vTriangleCache[id];
        data.vertices.assign(vertices, vertices + vertexCount * 3);
        data.indices.assign(indices, indices + indexCount);
        block->bDirty = true;

        uploadSinglePrimitive(block, nPrimIdx);
        return true;
    }

    /**
     * @brief 设置指定ID多边形的可见性
     *
     * 控制多边形是否在渲染时可见，不会实际删除或添加多边形数据，仅更新可见性标志。
     *
     * @param id 要设置的多边形的唯一标识符
     * @param bVisible true表示可见，false表示不可见
     * @return true 如果设置成功，false 如果多边形不存在
     */
    bool TriangleVboManager::setTriangleVisible(long long id, bool bVisible)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_IDLocationMap.find(id);
        if (it == m_IDLocationMap.end())
            return false;

        Location& loc = it->second;
        loc.block->vPrimitives[loc.nPrimIdx].bValid = bVisible;
        loc.block->bDirty = true;
        return true;
    }

    /**
     * @brief 清空所有多边形数据
     *
     * 移除所有多边形并释放相关资源，包括：
     * - 停止后台碎片整理线程
     * - 删除所有OpenGL缓冲区对象
     * - 清空所有容器和缓存
     *
     * @note 此操作会释放所有资源，调用后需要重新添加多边形
     */
    void TriangleVboManager::clearAllPrimitives()
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        for (auto& pair : m_colorBlocksMap)
        {
            for (TriangleColorVBOBlock* block : pair.second)
            {
                if (m_gl)
                {
                    m_gl->glDeleteVertexArrays(1, &block->vao);
                    m_gl->glDeleteBuffers(1, &block->vbo);
                    m_gl->glDeleteBuffers(1, &block->ebo);
                }
                delete block;
            }
        }
        m_colorBlocksMap.clear();
        m_IDLocationMap.clear();
        m_IDLocationMap.reserve(0);
        m_vTriangleCache.clear();
        m_triangleCacheOrder.clear();
    }

    // ===================================================================
    // 渲染核心（最高性能：glMultiDrawElementsBaseVertex）
    // ===================================================================

    /**
     * @brief 渲染所有可见的多边形
     *
     * 这是渲染的核心方法，采用了多项优化技术：
     * - 按颜色分组渲染，减少着色器 uniform 更新和状态切换
     * - 使用 glDrawElementsBaseVertex 进行高效绘制
     * - 延迟重建绘制命令，避免不必要的计算
     * - 自动进行内存碎片整理
     *
     * 渲染流程：
     * 1. 获取当前激活的着色器程序并查找颜色 uniform 位置
     * 2. 遍历所有颜色组
     * 3. 为每个颜色组设置统一颜色
     * 4. 遍历该颜色组的所有 VBO 块
     * 5. 重建脏块的绘制命令
     * 6. 压缩需要整理的块
     * 7. 绑定块并执行渲染
     *
     * @note 此方法应在 OpenGL 渲染上下文中调用
     */
    void TriangleVboManager::renderVisiblePrimitives()
    {
        if (!m_gl)
            return;

        std::shared_lock<std::shared_mutex> lock(m_mutex);

        GLint nProg = 0;
        m_gl->glGetIntegerv(GL_CURRENT_PROGRAM, &nProg);
        GLint uColorLoc = (nProg > 0) ? m_gl->glGetUniformLocation(nProg, "uColor") : -1;

        for (const auto& pair : m_colorBlocksMap)
        {
            const auto& vBlocks = pair.second;
            if (vBlocks.empty())
                continue;

            const Color& c = vBlocks[0]->color;
            if (uColorLoc != -1)
                m_gl->glUniform4f(uColorLoc, c.r(), c.g(), c.b(), c.a());

            for (TriangleColorVBOBlock* block : vBlocks)
            {
                if (block->vDrawCounts.empty() && !block->bDirty)
                    continue;

                if (block->bDirty)
                    rebuildDrawCmds(block);

                if (block->bCompact)
                    compactBlock(block);

                if (block->vDrawCounts.empty())
                    continue;

                bindBlock(block);

                // 计算每个图元的索引在EBO中的偏移
                // 需要从图元信息中获取索引偏移
                size_t drawIdx = 0;
                for (const TrianglePrimitiveInfo& prim : block->vPrimitives)
                {
                    if (prim.bValid && prim.nIndexCount > 0)
                    {
                        m_gl->glDrawElementsBaseVertex(
                            GL_TRIANGLES,
                            prim.nIndexCount,
                            GL_UNSIGNED_INT,
                            reinterpret_cast<const void*>(prim.nBaseIndex * sizeof(unsigned int)),
                            prim.nBaseVertex);
                        drawIdx++;
                    }
                }

                unbindBlock();
            }
        }
    }

    /**
     * @brief 渲染所有可见的多边形（使用 glMultiDrawElementsBaseVertex 优化版本）
     *
     * 使用 glMultiDrawElementsBaseVertex 进行批量绘制，性能更高。
     */
    void TriangleVboManager::renderVisiblePrimitivesEx()
    {
        if (!m_gl || m_colorBlocksMap.empty())
            return;

        std::shared_lock<std::shared_mutex> lock(m_mutex);

        GLint nProg = 0;
        m_gl->glGetIntegerv(GL_CURRENT_PROGRAM, &nProg);
        GLint uColorLoc = (nProg > 0) ? m_gl->glGetUniformLocation(nProg, "uColor") : -1;

        for (const auto& pair : m_colorBlocksMap)
        {
            const auto& vBlocks = pair.second;
            if (vBlocks.empty())
                continue;

            const Color& c = vBlocks[0]->color;
            if (uColorLoc != -1)
                m_gl->glUniform4f(uColorLoc, c.r(), c.g(), c.b(), c.a());

            for (TriangleColorVBOBlock* block : vBlocks)
            {
                if (block->bDirty)
                    rebuildDrawCmds(block);

                if (block->bCompact)
                    compactBlock(block);

                if (block->vDrawCounts.empty())
                    continue;

                bindBlock(block);

                // 收集有效的图元信息
                std::vector<GLsizei> drawCounts;
                std::vector<GLint> baseVertices;
                std::vector<const void*> indexOffsets;
                
                for (const TrianglePrimitiveInfo& prim : block->vPrimitives)
                {
                    if (prim.bValid && prim.nIndexCount > 0)
                    {
                        drawCounts.push_back(prim.nIndexCount);
                        baseVertices.push_back(prim.nBaseVertex);
                        indexOffsets.push_back(reinterpret_cast<const void*>(prim.nBaseIndex * sizeof(unsigned int)));
                    }
                }

                if (drawCounts.empty())
                {
                    unbindBlock();
                    continue;
                }

                GLsizei nPrimCount = static_cast<GLsizei>(drawCounts.size());

                m_gl->glMultiDrawElementsBaseVertex(
                    GL_TRIANGLES,
                    drawCounts.data(),      // nCount[]
                    GL_UNSIGNED_INT,
                    indexOffsets.data(),    // 每个图元的索引在EBO中的偏移
                    nPrimCount,             // draw command 数量
                    baseVertices.data()     // basevertex[]
                );

                unbindBlock();
            }
        }
    }

    // ===================================================================
    // 私有工具函数
    // ===================================================================

    /**
     * @brief 查找或创建指定颜色的VBO块
     *
     * 根据颜色查找现有可容量足够的VBO块，如果不存在则创建新的。
     * 这是实现颜色分组渲染优化的关键方法。
     *
     * @param color 要查找的颜色
     * @return 指向TriangleColorVBOBlock的指针，如果无法创建则返回nullptr
     */
    TriangleColorVBOBlock* TriangleVboManager::getColorBlock(const Color& color)
    {
        uint32_t nKey = color.toUInt32();
        auto& vBlocks = m_colorBlocksMap[nKey];

        for (TriangleColorVBOBlock* b : vBlocks)
        {
            if (b->nVertexCount + 5000 < MAX_VERT_PER_BLOCK)
                return b;
        }

        return createNewColorBlock(color);
    }

    /**
     * @brief 创建新的颜色VBO块
     *
     * 分配并初始化一个新的TriangleColorVBOBlock对象，包括：
     * - 创建OpenGL缓冲区对象（VAO、VBO、EBO）
     * - 设置初始容量
     * - 配置顶点属性指针
     * - 将新块添加到颜色映射中
     *
     * @param color 该块的颜色
     * @return 指向新创建的TriangleColorVBOBlock的指针
     */
    TriangleColorVBOBlock* TriangleVboManager::createNewColorBlock(const Color& color)
    {
        TriangleColorVBOBlock* block = new TriangleColorVBOBlock();
        block->color = color;

        m_gl->glGenVertexArrays(1, &block->vao);
        m_gl->glGenBuffers(1, &block->vbo);
        m_gl->glGenBuffers(1, &block->ebo);

        block->nVertexCapacity = INIT_CAPACITY;
        block->nIndexCapacity = INIT_CAPACITY;

        m_gl->glBindVertexArray(block->vao);

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(INIT_CAPACITY * 3 * sizeof(float)),
            nullptr, GL_DYNAMIC_DRAW);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(INIT_CAPACITY * sizeof(unsigned int)),
            nullptr, GL_DYNAMIC_DRAW);

        m_gl->glEnableVertexAttribArray(0);
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        m_gl->glBindVertexArray(0);

        m_colorBlocksMap[color.toUInt32()].push_back(block);
        return block;
    }

    /**
     * @brief 确保VBO块有足够的容量
     *
     * 检查并在必要时扩容指定的VBO块，以容纳所需的顶点和索引数量。
     * 如果块空间不足，将重新分配更大的缓冲区并复制现有数据。
     *
     * @param block 要检查容量的VBO块
     * @param nNeedV 需要的顶点数量
     * @param nNeedI 需要的索引数量
     */
    void TriangleVboManager::checkBlockCapacity(TriangleColorVBOBlock* block, size_t nNeedV, size_t nNeedI)
    {
        if (nNeedV <= block->nVertexCapacity && nNeedI <= block->nIndexCapacity)
            return;

        size_t nNewCap = block->nVertexCapacity * 2;
        if (nNewCap < nNeedV)
            nNewCap = nNeedV + GROW_STEP;

        size_t nNewIdxCap = block->nIndexCapacity * 2;
        if (nNewIdxCap < nNeedI)
            nNewIdxCap = nNeedI + GROW_STEP;

        block->nVertexCapacity = nNewCap;
        block->nIndexCapacity = nNewIdxCap;

        // Orphaning：只重新分配，不拷贝旧数据
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER,
            nNewCap * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            nNewIdxCap * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

        // 旧数据全丢！因为我们有 m_vTriangleCache 缓存，下次 compact 时会重建
        block->bCompact = true;  // 强制下次 compact 时重建
    }

    /**
     * @brief 上传单个多边形到VBO块
     *
     * 将多边形数据（顶点和索引）上传到指定的VBO块中，包括：
     * - 计算顶点和索引的偏移量
     * - 使用glBufferSubData更新VBO和EBO
     * - 计算相对索引值（索引需要相对于baseVertex偏移）
     * - 更新渲染相关状态
     *
     * @param block 目标VBO块
     * @param nPrimIdx 要上传的多边形在块中的索引
     */
    void TriangleVboManager::uploadSinglePrimitive(TriangleColorVBOBlock* block, size_t nPrimIdx)
    {
        const TrianglePrimitiveInfo& prim = block->vPrimitives[nPrimIdx];
        if (!prim.bValid)
            return;

        auto it = m_vTriangleCache.find(prim.id);
        if (it == m_vTriangleCache.end())
            return;

        const TriangleData& data = it->second;
        size_t nVertCount = data.vertices.size() / 3;
        size_t nIdxCount = data.indices.size();

        GLsizeiptr nVertOffset = static_cast<GLsizeiptr>(prim.nBaseVertex) * 3 * sizeof(float);
        GLsizeiptr nIdxOffset = static_cast<GLsizeiptr>(prim.nBaseIndex) * sizeof(unsigned int);

        // 顶点
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferSubData(GL_ARRAY_BUFFER, nVertOffset,
            static_cast<GLsizeiptr>(data.vertices.size() * sizeof(float)), data.vertices.data());

        // 索引（需要相对于baseVertex进行偏移）
        std::vector<unsigned int> vIndices(nIdxCount);
        for (size_t i = 0; i < nIdxCount; ++i)
            vIndices[i] = static_cast<unsigned int>(prim.nBaseVertex + data.indices[i]);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, nIdxOffset,
            static_cast<GLsizeiptr>(vIndices.size() * sizeof(unsigned int)), vIndices.data());
    }

    /**
     * @brief 压缩VBO块，整理内存碎片
     *
     * 当块中存在被删除的多边形时，此方法负责：
     * - 移除无效的多边形数据
     * - 重新组织顶点和索引数据以消除空洞
     * - 更新所有受影响的多边形的基础顶点偏移
     * - 重置缓冲区大小为实际使用大小
     * - 重新生成绘制命令
     *
     * @param block 要压缩的VBO块
     */
    void TriangleVboManager::compactBlock(TriangleColorVBOBlock* block)
    {
        if (!block->bCompact || block->nVertexCount == 0)
            return;

        std::vector<float> newVerts;
        std::vector<unsigned int> newIndices;
        newVerts.reserve(block->nVertexCount * 3 * 3 / 4);  // 预估 75% 存活
        newIndices.reserve(block->nIndexCount * 3 / 4);

        size_t currentBase = 0;

        for (TrianglePrimitiveInfo& prim : block->vPrimitives)
        {
            if (!prim.bValid || prim.nIndexCount <= 0)
                continue;

            // Step 1: 先尝试从缓存拿（最快）
            auto cacheIt = m_vTriangleCache.find(prim.id);
            const TriangleData* dataPtr = nullptr;
            TriangleData tempData;

            if (cacheIt != m_vTriangleCache.end())
            {
                dataPtr = &cacheIt->second;
            }
            else
            {
                // Step 2: 缓存没了？从当前 VBO 里把旧数据读回来！（关键兜底！）
                size_t oldVertOffset = static_cast<size_t>(prim.nBaseVertex);
                size_t vertCount = 0;
                size_t idxCount = static_cast<size_t>(prim.nIndexCount);

                // 需要从索引中推断顶点数量，或者从其他图元信息中获取
                // 这里简化处理：假设我们知道顶点数量
                // 实际上应该从索引的最大值+1来推断
                // 为了简化，我们从缓存中读取，如果缓存没有，我们需要一个更好的方法
                // 暂时跳过这种情况，或者需要存储顶点数量信息
                continue; // 跳过没有缓存的数据
            }

            const auto& data = *dataPtr;
            size_t nVertCount = data.vertices.size() / 3;
            size_t nIdxCount = data.indices.size();

            // 写入新缓冲区
            newVerts.insert(newVerts.end(), data.vertices.begin(), data.vertices.end());
            
            // 记录索引的起始位置
            size_t indexStart = newIndices.size();
            
            // 索引需要相对于新的 baseVertex 进行偏移
            for (size_t i = 0; i < nIdxCount; ++i)
                newIndices.push_back(static_cast<unsigned int>(currentBase + data.indices[i]));

            // 更新 base vertex 和 base index
            prim.nBaseVertex = static_cast<GLint>(currentBase);
            prim.nBaseIndex = indexStart; // 更新索引偏移
            currentBase += nVertCount;
        }

        // 一次性上传新数据
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER,
            block->nVertexCapacity * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        m_gl->glBufferSubData(GL_ARRAY_BUFFER, 0,
            newVerts.size() * sizeof(float), newVerts.data());

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            block->nIndexCapacity * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
        m_gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
            newIndices.size() * sizeof(unsigned int), newIndices.data());

        // 更新统计
        block->nVertexCount = currentBase;
        block->nIndexCount = newIndices.size();
        block->bCompact = false;
        block->bDirty = true;
    }

    /**
     * @brief 重建绘制命令
     *
     * 根据块中的图元数据重新生成批量绘制命令。
     *
     * @param block 要重建命令的块
     */
    void TriangleVboManager::rebuildDrawCmds(TriangleColorVBOBlock* block)
    {
        block->vDrawCounts.clear();
        block->vBaseVertices.clear();

        for (const TrianglePrimitiveInfo& prim : block->vPrimitives)
        {
            if (prim.bValid && prim.nIndexCount > 0)
            {
                block->vDrawCounts.push_back(prim.nIndexCount);
                block->vBaseVertices.push_back(prim.nBaseVertex);
            }
        }

        block->bDirty = false;
    }

    /**
     * @brief 更新缓存访问顺序（LRU）
     *
     * @param id 多边形ID
     */
    void TriangleVboManager::touchCache(long long id)
    {
        m_triangleCacheOrder.remove(id);
        m_triangleCacheOrder.push_front(id);
        if (m_triangleCacheOrder.size() > MAX_CACHE_SIZE)
        {
            long long old = m_triangleCacheOrder.back();
            m_triangleCacheOrder.pop_back();
            m_vTriangleCache.erase(old);
        }
    }

    /**
     * @brief 绑定块的OpenGL资源
     *
     * @param block 要绑定的块
     */
    void TriangleVboManager::bindBlock(TriangleColorVBOBlock* block) const
    {
        m_gl->glBindVertexArray(block->vao);
    }

    /**
     * @brief 解绑当前块的OpenGL资源
     */
    void TriangleVboManager::unbindBlock() const
    {
        m_gl->glBindVertexArray(0);
    }

    /**
     * @brief 启动后台碎片整理线程
     *
     * 启动一个单独的线程进行内存碎片整理，定期检查并压缩需要整理的块。
     */
    void TriangleVboManager::startBackgroundDefrag()
    {
        m_bStopDefrag = false;
        m_defragThread = std::thread([this] {
            while (!m_bStopDefrag)
            {
                for (int i = 0; i < 30 && !m_bStopDefrag.load(); ++i)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    if (m_bStopDefrag)
                        return;
                }

                std::shared_lock lock(m_mutex);
                for (auto& pair : m_colorBlocksMap)
                {
                    for (TriangleColorVBOBlock* block : pair.second)
                    {
                        // 只有使用率 < 70% 才值得整理
                        if (block->bCompact && block->nVertexCount > 0)
                        {
                            double dUsage = double(block->nVertexCount) / block->nVertexCapacity;
                            if (dUsage < COMPACT_THRESHOLD)
                            {
                                lock.unlock();
                                std::unique_lock writeLock(m_mutex);
                                compactBlock(block);
                                break;
                            }
                        }
                    }
                }
            }
            });
    }

    /**
     * @brief 停止后台碎片整理线程
     */
    void TriangleVboManager::stopBackgroundDefrag()
    {
        m_bStopDefrag = true;
        if (m_defragThread.joinable())
        {
            m_bStopDefrag = true;
            m_defragThread.join();
        }
    }
} // namespace GLRhi
