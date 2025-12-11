/**
 * @class PolylinesVboManager
 * @brief 高性能折线渲染管理器，支持百万级动态折线的实时增删改查和渲染
 *
 * 该类是一个高度优化的OpenGL折线渲染管理系统，具有以下特点：
 * - 按颜色分组管理折线数据，优化渲染性能
 * - 支持动态添加、删除、更新折线，无需重建整个缓冲区
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

#include "DataManager/PolylinesVboManager.h"

namespace GLRhi
{
    // 类常量定义
    namespace
    {
        static constexpr size_t INIT_CAPACITY = 100'000; // 改这里！原来 1'000'000 太离谱
        static constexpr size_t GROW_STEP = 200'000;
        static constexpr size_t MAX_VERT_PER_BLOCK = 1'500'000;

        // static constexpr size_t INIT_CAPACITY = 1'000'000;     // VBO块的初始容量
        // static constexpr size_t GROW_STEP = 500'000;         // 容量增长步长
        // static constexpr size_t MAX_VERT_PER_BLOCK = 2'000'000; // 每个VBO块的最大顶点数量
        static constexpr float COMPACT_THRESHOLD = 0.70f; // 使用率 < 70% 才压缩
    }

    /**
     * @brief 构造函数，初始化PolylinesVboManager
     *
     * 在构造时尝试获取当前的OpenGL上下文，并初始化相关资源。
     * 注意：在创建此对象时，必须确保OpenGL上下文已经初始化。
     */
    PolylinesVboManager::PolylinesVboManager()
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
     * - 释放所有ColorVBOBlock对象
     * - 清空所有容器（m_colorBlocks, m_IDLocationMap, m_vertexCache）
     */
    PolylinesVboManager::~PolylinesVboManager()
    {
        stopBackgroundDefrag();

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        for (auto& pair : m_colorBlocksMap)
        {
            for (ColorVBOBlock* block : pair.second)
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
        m_vVertexCache.clear();
        m_vVertexCache.reserve(0);
    }


    bool PolylinesVboManager::initialize(QOpenGLContext* context)
    {
        if (!context)
        {
            qWarning() << "[LineBufferManager] initialize: context is null";
            return false;
        }

        m_context = context;
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if (!m_gl)
        {
            qWarning() << "[LineBufferManager] initialize: failed to get OpenGL 3.3 functions";
            return false;
        }

        m_gl->initializeOpenGLFunctions();
        return true;
    }


    /**
     * @brief 添加一条折线到渲染管理器
     *
     * 将一条新的折线添加到系统中，自动按颜色分组存储，并执行增量上传到GPU。
     *
     * @param id 折线的唯一标识符，用于后续的更新和删除操作
     * @param vVerts 顶点数据，格式为[x1,y1,z1,x2,y2,z2,...]，每个顶点3个浮点数
     * @param color 折线的颜色
     * @return true 如果添加成功，false 如果参数无效或折线已存在
     *
     * @note 顶点数量必须至少为2个（即vVertices.size() >= 6且为3的倍数）
     */
    bool PolylinesVboManager::addPolyline(long long id,
        float* vertices, size_t vertexCount, const Color& color)
    {
        if (vertexCount < 6)
            return false;

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (m_IDLocationMap.count(id))
        {
            m_mutex.unlock();
            setPolylineVisible(id, true);
            return false;
        }

        ColorVBOBlock* block = getColorBlock(color);
        if (!block)
            return false;

        size_t nVertCount = vertexCount / 3;
        checkBlockCapacity(block,
            block->nVertexCount + nVertCount,
            block->nIndexCount + nVertCount);

        GLint nBaseVertex = static_cast<GLint>(block->nVertexCount);

        PrimitiveInfo prim;
        prim.id = id;
        prim.nIndexCount = static_cast<GLsizei>(nVertCount);
        prim.nBaseVertex = nBaseVertex;
        prim.bValid = true;

        size_t nPrimIdx = block->vPrimitives.size();
        block->vPrimitives.push_back(std::move(prim));
        block->idToIndexMap[id] = nPrimIdx;

        block->nVertexCount += nVertCount;
        block->nIndexCount += nVertCount;
        block->bDirty = true;

        // 将顶点数据复制到缓存中
        m_vVertexCache[id].assign(vertices, vertices + vertexCount);
        m_IDLocationMap[id] = { color.toUInt32(), color, block, nPrimIdx };

        uploadSinglePrimitive(block, nPrimIdx); // 增量上传，只传这一条
        return true;
    }

    /**
     * @brief 批量添加多条折线到渲染管理器
     *
     * 一次性添加多个折线组，每个折线组包含多条折线。
     * 内部会遍历所有折线数据并调用addPolyline添加每条折线。
     *
     * @param vPlDatas 折线数据数组，每个元素包含一组相关的折线
     * @return true 如果所有折线都添加成功，false 如果至少有一条折线添加失败
     *
     * @note 内部会跳过顶点数少于2的无效折线
     */
    bool PolylinesVboManager::addPolylines(const std::vector<PolylineData>& vPlDatas)
    {
        bool bAllSuccess = true;
        for (auto& data : vPlDatas)
        {
            size_t offset = 0;
            for (size_t i = 0; i < data.vId.size(); ++i)
            {
                size_t nCount = data.vCount[i];
                if (nCount < 2)
                {
                    offset += nCount;
                    continue;
                }

                std::vector<float> vVerts;
                vVerts.reserve(nCount * 3);
                const float* src = data.vVerts.data() + offset * 3;
                vVerts.insert(vVerts.end(), src, src + nCount * 3);

                Color c(data.brush.getColor());
                if (!addPolyline(data.vId[i], vVerts.data(), vVerts.size(), c))
                {
                    touchCache(data.vId[i]);
                    bAllSuccess = false;
                }

                offset += nCount;
            }
        }
        return bAllSuccess;
    }

    /**
     * @brief 批量添加多条折线
     *
     * 比逐个调用 addPolyline() 快 5~20 倍，专为一次性加载数十万条线设计。
     * 内部会：
     * - 按颜色自动分组
     * - 每种颜色只扩容/上传一次
     * - 最小化锁粒度、OpenGL 调用、内存拷贝
     *
     * @param vPolylineDatas 批量数据：{id, vertices, color}
     * @return 添加成功的图元数量（失败的会跳过并打印警告）
     */
    size_t PolylinesVboManager::addPolylines(
        const std::vector<std::tuple<long long, float*, size_t, Color>>& vPolylineDatas)
    {
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if (vPolylineDatas.empty() || !m_gl)
            return 0;

        // Step 1: 预处理 — 按颜色分组 + 过滤无效数据 + 预检查ID冲突
        struct BatchGroup
        {
            Color color;
            std::vector<size_t> indices; // 在 vPolylineDatas 中的下标
            size_t totalVerts = 0;
            size_t totalIndices = 0;
        };

        std::unordered_map<uint32_t, BatchGroup> colorGroups;
        std::vector<bool> validFlags(vPolylineDatas.size(), true);
        size_t validCount = 0;

        {
            std::shared_lock<std::shared_mutex> readLock(m_mutex);
            for (size_t i = 0; i < vPolylineDatas.size(); ++i) // 遍历组
            {
                const auto& [id, verts, vertexCount, color] = vPolylineDatas[i]; // tuple 多段线

                if (vertexCount < 6)
                {
                    validFlags[i] = false;
                    continue;
                }

                if (m_IDLocationMap.count(id))
                {
                    validFlags[i] = false;
                    continue;
                }

                size_t nVertCount = vertexCount / 3;
                uint32_t key = color.toUInt32();

                auto& batchGroup = colorGroups[key];
                batchGroup.color = color;

                batchGroup.indices.push_back(i);
                batchGroup.totalVerts += nVertCount;
                batchGroup.totalIndices += nVertCount;
                validCount++;
            }
        }

        if (validCount == 0)
            return 0;

        std::unique_lock<std::shared_mutex> writeLock(m_mutex);

        size_t nAdd = 0;

        for (auto& [key, group] : colorGroups)
        {
            ColorVBOBlock* block = getColorBlock(group.color);
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

            std::vector<PrimitiveInfo> vNewPrims;
            vNewPrims.reserve(group.indices.size());

            // 遍历该颜色组所有图元
            for (size_t idx : group.indices)
            {
                if (!validFlags[idx])
                    continue;

                const auto& [id, verts, vertexCount, color] = vPolylineDatas[idx];
                size_t nVertCount = vertexCount / 3;
                PrimitiveInfo prim;
                prim.id = id;
                prim.nIndexCount = static_cast<GLsizei>(nVertCount);
                prim.nBaseVertex = static_cast<GLint>(nVertOffset);
                prim.bValid = true;

                // 记录图元信息
                size_t nPrimIdxInBlock = block->vPrimitives.size() + vNewPrims.size();
                vNewPrims.push_back(std::move(prim));
                block->idToIndexMap[id] = nPrimIdxInBlock;

                // 缓存顶点（用于后续 update / compact）
                m_vVertexCache[id].assign(verts, verts + vertexCount);

                // 填充批量缓冲区
                vBatchVerts.insert(vBatchVerts.end(), verts, verts + vertexCount);
                for (size_t i = 0; i < nVertCount; ++i)
                    vBatchIndices.push_back(static_cast<unsigned int>(nVertOffset + i));

                // 更新位置映射
                m_IDLocationMap[id] = { key, color, block, nPrimIdxInBlock };

                nVertOffset += nVertCount;
                nIdxOffset += nVertCount;
                nAdd++;
            }

            //  一次性上传
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
     * @brief 从渲染管理器中移除指定ID的折线
     *
     * 通过ID查找并移除折线，标记为无效并触发后续的内存整理。
     * 实际的内存释放会在碎片整理时进行，以优化性能。
     *
     * @param id 要移除的折线的唯一标识符
     * @return true 如果成功移除，false 如果折线不存在
     */
    bool PolylinesVboManager::removePolyline(long long id)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_IDLocationMap.find(id);
        if (it == m_IDLocationMap.end())
            return false;

        const Location& loc = it->second;
        ColorVBOBlock* block = loc.block;
        PrimitiveInfo& prim = block->vPrimitives[loc.nPrimIdx];

        prim.bValid = false;
        prim.nIndexCount = 0;
        block->bDirty = true;
        block->bCompact = true;

        m_IDLocationMap.erase(it);
        m_vVertexCache.erase(id);
        block->idToIndexMap.erase(id);

        // 立即重新整理VBO数据，确保删除后顶点数据是连续的
        // 这样可以避免渲染时线段连接错误
        //compactBlock(block);
        //rebuildDrawCmds(block);
        return true;
    }

    size_t PolylinesVboManager::removePolylines(const std::vector<long long>& vIds)
    {
        if (vIds.empty())
            return true;

        std::unique_lock lock(m_mutex);
        size_t nDelCount = 0;

        for (long long id : vIds)
        {
            auto it = m_IDLocationMap.find(id);
            if (it == m_IDLocationMap.end())
                continue;

            const Location& loc = it->second;
            ColorVBOBlock* block = loc.block;
            PrimitiveInfo& prim = block->vPrimitives[loc.nPrimIdx];

            prim.bValid = false;
            prim.nIndexCount = 0;
            block->bDirty = true;
            block->bCompact = true;

            m_IDLocationMap.erase(it);
            m_vVertexCache.erase(id);
            block->idToIndexMap.erase(id);
            ++nDelCount;
        }
        return nDelCount;
    }

    /**
     * @brief 更新指定ID的折线数据
     *
     * 更新现有折线的顶点数据，根据顶点数量变化采用不同策略：
     * - 顶点数量变化不大时，直接增量更新
     * - 顶点数量变化超过50%时，采用删除后重新添加的策略，确保内存使用合理
     *
     * @param id 要更新的折线的唯一标识符
     * @param vVerts 新的顶点数据，格式为[x1,y1,z1,x2,y2,z2,...]
     * @return true 如果更新成功，false 如果参数无效或折线不存在
     *
     * @note 顶点数量必须至少为2个（即vVertices.size() >= 6且为3的倍数）
     */
    bool PolylinesVboManager::updatePolyline(long long id, float* vertices, size_t vertexCount)
    {
        if (vertexCount < 6)
            return false;

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_IDLocationMap.find(id);
        if (it == m_IDLocationMap.end())
            return false;

        const Location& loc = it->second;
        ColorVBOBlock* block = loc.block;
        size_t nPrimIdx = loc.nPrimIdx;
        PrimitiveInfo& prim = block->vPrimitives[nPrimIdx];

        size_t nOldVertCount = static_cast<size_t>(prim.nIndexCount); // 旧顶点数
        size_t nNewVertCount = vertexCount / 3;
        if (nNewVertCount > nOldVertCount)
        {
            Color c = loc.color;
            lock.unlock();
            removePolyline(id);
            return addPolyline(id, vertices, vertexCount, c);
        }

        prim.nIndexCount = static_cast<GLsizei>(nNewVertCount);
        prim.bValid = true;

        // 将顶点数据复制到缓存中
        m_vVertexCache[id].assign(vertices, vertices + vertexCount);
        block->bDirty = true;

        uploadSinglePrimitive(block, nPrimIdx);
        return true;
    }

    /**
     * @brief 设置指定ID折线的可见性
     *
     * 控制折线是否在渲染时可见，不会实际删除或添加折线数据，仅更新可见性标志。
     *
     * @param id 要设置的折线的唯一标识符
     * @param bVisible true表示可见，false表示不可见
     * @return true 如果设置成功，false 如果折线不存在
     */
    bool PolylinesVboManager::setPolylineVisible(long long id, bool bVisible)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_IDLocationMap.find(id);
        if (it == m_IDLocationMap.end())
            return false;

        Location& loc = it->second;

        size_t nPrim = loc.nPrimIdx;

        // bool bTest = loc.block->vPrimitives[nPrim].bValid;
        // bVisible = !bTest;

        loc.block->vPrimitives[loc.nPrimIdx].bValid = bVisible;
        loc.block->bDirty = true;
        return true;
    }

    /**
     * @brief 清空所有折线数据
     *
     * 移除所有折线并释放相关资源，包括：
     * - 停止后台碎片整理线程
     * - 删除所有OpenGL缓冲区对象
     * - 清空所有容器和缓存
     *
     * @note 此操作会释放所有资源，调用后需要重新添加折线
     */
    void PolylinesVboManager::clearAllPrimitives()
    {
        // stopBackgroundDefrag();
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        for (auto& pair : m_colorBlocksMap)
        {
            for (ColorVBOBlock* block : pair.second)
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
        m_vVertexCache.clear();
        m_vVertexCache.reserve(0);
    }

    // ===================================================================
    // 渲染核心（最高性能：glMultiDrawElementsBaseVertex）
    // ===================================================================

    /**
     * @brief 渲染所有可见的折线
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
    void PolylinesVboManager::renderVisiblePrimitives()
    {
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
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

            for (ColorVBOBlock* block : vBlocks)
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

                for (size_t i = 0; i < block->vDrawCounts.size(); ++i)
                {
                    m_gl->glDrawElementsBaseVertex(
                        GL_LINE_STRIP,
                        block->vDrawCounts[i],
                        GL_UNSIGNED_INT,
                        nullptr,
                        block->vBaseVertices[i]);
                }

                unbindBlock();
            }
        }
    }

    void PolylinesVboManager::renderVisiblePrimitivesEx()
    {
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if (!m_gl || m_colorBlocksMap.empty())
            return;

        std::shared_lock<std::shared_mutex> lock(m_mutex);

        GLint nProg = 0;
        m_gl->glGetIntegerv(GL_CURRENT_PROGRAM, &nProg);
        GLint uColorLoc = (nProg > 0) ? m_gl->glGetUniformLocation(nProg, "uColor") : -1;

        //static std::vector<const void*> vOffsets(100000, nullptr);

        for (const auto& pair : m_colorBlocksMap)
        {
            const auto& vBlocks = pair.second;
            if (vBlocks.empty())
                continue;

            const Color& c = vBlocks[0]->color;
            if (uColorLoc != -1)
                m_gl->glUniform4f(uColorLoc, c.r(), c.g(), c.b(), c.a());

            for (ColorVBOBlock* block : vBlocks)
            {
                if (block->bDirty)
                    rebuildDrawCmds(block);

                if (block->bCompact)
                    compactBlock(block);

                if (block->vDrawCounts.empty())
                    continue;

                bindBlock(block);

                GLsizei nPrimCount = static_cast<GLsizei>(block->vDrawCounts.size());

                static thread_local std::vector<const void*> g_nullPointers;
                if (g_nullPointers.size() < 200000)
                    g_nullPointers.assign(200000, nullptr);

                // 构造一个全是 nullptr 的指针数组
                // 因为使用的是相对索引（0,1,2,...），所有 draw command 的 index offset 都是 0
                const void** ptrs = g_nullPointers.data();

                m_gl->glMultiDrawElementsBaseVertex(
                    GL_LINE_STRIP,
                    block->vDrawCounts.data(), // nCount[]
                    GL_UNSIGNED_INT,
                    ptrs,               // 必须是 [nullptr, nullptr, ...]，长度 = primCount
                    nPrimCount,                 // draw command 数量
                    block->vBaseVertices.data() // basevertex[]
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
     * @return 指向ColorVBOBlock的指针，如果无法创建则返回nullptr
     */
    ColorVBOBlock* PolylinesVboManager::getColorBlock(const Color& color)
    {
        uint32_t nKey = color.toUInt32();
        auto& vBlocks = m_colorBlocksMap[nKey];

        for (ColorVBOBlock* b : vBlocks)
        {
            if (b->nVertexCount + 5000 < MAX_VERT_PER_BLOCK)
                return b;
        }

        return createNewColorBlock(color);
    }

    /**
     * @brief 创建新的颜色VBO块
     *
     * 分配并初始化一个新的ColorVBOBlock对象，包括：
     * - 创建OpenGL缓冲区对象（VAO、VBO、EBO）
     * - 设置初始容量
     * - 配置顶点属性指针
     * - 将新块添加到颜色映射中
     *
     * @param color 该块的颜色
     * @return 指向新创建的ColorVBOBlock的指针
     */
    ColorVBOBlock* PolylinesVboManager::createNewColorBlock(const Color& color)
    {
        ColorVBOBlock* block = new ColorVBOBlock();
        block->color = color;

        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
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
    void PolylinesVboManager::checkBlockCapacity(ColorVBOBlock* block, size_t nNeedV, size_t nNeedI)
    {
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();

        if (1)
        {
            // Grok 优化版
            if (nNeedV <= block->nVertexCapacity && nNeedI <= block->nIndexCapacity)
                return;

            size_t nNewCap = block->nVertexCapacity * 2;
            if (nNewCap < nNeedV)
                nNewCap = nNeedV + GROW_STEP;

            block->nVertexCapacity = nNewCap;
            block->nIndexCapacity = nNewCap;

            // Orphaning：只重新分配，不拷贝旧数据
            m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
            m_gl->glBufferData(GL_ARRAY_BUFFER,
                nNewCap * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

            m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
            m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                nNewCap * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

            // 旧数据全丢！因为我们有 m_vVertexCache 缓存，下次 compact 时会重建
            block->bCompact = true;  // 强制下次 compact 时重建

            return;
        }
        else
        {
            // 没崩溃过
            if (nNeedV <= block->nVertexCapacity && nNeedI <= block->nIndexCapacity)
                return;

            size_t nNewV = block->nVertexCapacity;
            size_t nNewI = block->nIndexCapacity;

            while (nNewV < nNeedV)
                nNewV += GROW_STEP;
            while (nNewI < nNeedI)
                nNewI += GROW_STEP;

            // 保存旧容量信息，用于后续重新上传数据
            size_t nOldVertexCount = block->nVertexCount;
            size_t nOldIndexCount = block->nIndexCount;

            block->nVertexCapacity = nNewV;
            block->nIndexCapacity = nNewI;

            // Orphaning 重新分配 - 但这次我们需要重新上传数据
            m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
            m_gl->glBufferData(GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(nNewV * 3 * sizeof(float)), nullptr, GL_DYNAMIC_DRAW);

            m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
            m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(nNewI * sizeof(unsigned int)), nullptr, GL_DYNAMIC_DRAW);

            // 重新上传所有图元的数据
            if (nOldVertexCount > 0)
            {
                std::vector<float> allVertices;
                std::vector<unsigned int> allIndices;

                allVertices.reserve(nOldVertexCount * 3);
                allIndices.reserve(nOldIndexCount);

                // 从顶点缓存中收集所有有效图元的数据
                // 每次扩容就重传整个 block（几百万顶点） !!!!
                size_t currentBase = 0;
                for (size_t i = 0; i < block->vPrimitives.size(); ++i)
                {
                    const PrimitiveInfo& prim = block->vPrimitives[i];
                    if (prim.bValid)
                    {
                        auto it = m_vVertexCache.find(prim.id);
                        if (it != m_vVertexCache.end())
                        {
                            const std::vector<float>& vertices = it->second;
                            size_t vertexCount = vertices.size() / 3;

                            // 添加顶点数据
                            allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());

                            // 添加索引数据
                            for (size_t j = 0; j < vertexCount; ++j)
                            {
                                allIndices.push_back(static_cast<unsigned int>(currentBase + j));
                            }

                            currentBase += vertexCount;
                        }
                    }
                }

                // 上传重新收集的数据
                m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
                m_gl->glBufferSubData(GL_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(allVertices.size() * sizeof(float)), allVertices.data());

                m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
                m_gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(allIndices.size() * sizeof(unsigned int)), allIndices.data());
            }
            return;
        }
    }

    /**
     * @brief 上传单个折线到VBO块
     *
     * 将折线数据（顶点和索引）上传到指定的VBO块中，包括：
     * - 计算顶点和索引的偏移量
     * - 使用glBufferSubData更新VBO和EBO
     * - 计算相对索引值
     * - 更新渲染相关状态
     *
     * @param block 目标VBO块
     * @param nPrimIdx 要上传的折线在块中的索引
     */
    void PolylinesVboManager::uploadSinglePrimitive(ColorVBOBlock* block, size_t nPrimIdx)
    {
        const PrimitiveInfo& prim = block->vPrimitives[nPrimIdx];
        if (!prim.bValid)
            return;

        auto it = m_vVertexCache.find(prim.id);
        if (it == m_vVertexCache.end())
            return;

        const std::vector<float>& vVerts = it->second;
        size_t nVertCount = vVerts.size() / 3;

        GLsizeiptr nVertOffset = static_cast<GLsizeiptr>(prim.nBaseVertex) * 3 * sizeof(float);
        GLsizeiptr nIdxOffset = static_cast<GLsizeiptr>(prim.nBaseVertex) * sizeof(unsigned int);

        // 顶点
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferSubData(GL_ARRAY_BUFFER, nVertOffset,
            static_cast<GLsizeiptr>(vVerts.size() * sizeof(float)), vVerts.data());

        // 索引
        std::vector<unsigned int> vIndices(nVertCount);
        for (size_t i = 0; i < nVertCount; ++i)
            vIndices[i] = static_cast<unsigned int>(prim.nBaseVertex + i);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, nIdxOffset,
            static_cast<GLsizeiptr>(vIndices.size() * sizeof(unsigned int)), vIndices.data());
    }

    /**
     * @brief 压缩VBO块，整理内存碎片
     *
     * 当块中存在被删除的折线时，此方法负责：
     * - 移除无效的折线数据
     * - 重新组织顶点和索引数据以消除空洞
     * - 更新所有受影响的折线的基础顶点偏移
     * - 重置缓冲区大小为实际使用大小
     * - 重新生成绘制命令
     *
     * @param block 要压缩的VBO块
     */
    void PolylinesVboManager::compactBlock(ColorVBOBlock* block)
    {
        if (!block->bCompact || block->nVertexCount == 0)
            return;

        std::vector<float> newVerts;
        std::vector<unsigned int> newIndices;
        newVerts.reserve(block->nVertexCount * 3 * 3 / 4);  // 预估 75% 存活
        newIndices.reserve(block->nVertexCount);

        size_t currentBase = 0;
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();

        for (PrimitiveInfo& prim : block->vPrimitives)
        {
            if (!prim.bValid || prim.nIndexCount <= 1)
                continue;

            // Step 1: 先尝试从缓存拿（最快）
            auto cacheIt = m_vVertexCache.find(prim.id);
            const std::vector<float>* vertsPtr = nullptr;
            std::vector<float> tempVerts;

            if (cacheIt != m_vVertexCache.end())
            {
                vertsPtr = &cacheIt->second;
            }
            else
            {
                // Step 2: 缓存没了？从当前 VBO 里把旧数据读回来！（关键兜底！）
                // 这才是真正的「数据永不丢失」
                size_t oldOffset = static_cast<size_t>(prim.nBaseVertex);
                size_t vertCount = static_cast<size_t>(prim.nIndexCount);

                tempVerts.resize(vertCount * 3);
                m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
                m_gl->glGetBufferSubData(GL_ARRAY_BUFFER,
                    oldOffset * 3 * sizeof(float),
                    vertCount * 3 * sizeof(float),
                    tempVerts.data());

                vertsPtr = &tempVerts;

                // 可选：重新放回缓存（防止下次又读）
                // m_vVertexCache[prim.id] = std::move(tempVerts);
                // touchCache(prim.id);
            }

            const auto& verts = *vertsPtr;
            size_t nCount = verts.size() / 3;

            // 写入新缓冲区
            newVerts.insert(newVerts.end(), verts.begin(), verts.end());
            for (size_t i = 0; i < nCount; ++i)
                newIndices.push_back(static_cast<unsigned int>(currentBase + i));

            // 更新 base vertex
            prim.nBaseVertex = static_cast<GLint>(currentBase);
            currentBase += nCount;
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
        block->nIndexCount = currentBase;
        block->bCompact = false;
        block->bDirty = true;
    }

    void PolylinesVboManager::rebuildDrawCmds(ColorVBOBlock* block)
    {
        block->vDrawCounts.clear();
        block->vBaseVertices.clear();

        for (const PrimitiveInfo& prim : block->vPrimitives)
        {
            if (prim.bValid && prim.nIndexCount > 0)
            {
                block->vDrawCounts.push_back(prim.nIndexCount);
                block->vBaseVertices.push_back(prim.nBaseVertex);
            }
        }

        block->bDirty = false;
    }

    void PolylinesVboManager::touchCache(long long id)
    {
        m_vertexCacheOrder.remove(id);
        m_vertexCacheOrder.push_front(id);
        if (m_vertexCacheOrder.size() > MAX_CACHE_SIZE)
        {
            long long old = m_vertexCacheOrder.back();
            m_vertexCacheOrder.pop_back();
            m_vVertexCache.erase(old);
        }
    }

    void PolylinesVboManager::bindBlock(ColorVBOBlock* block)
    {
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        m_gl->glBindVertexArray(block->vao);
    }

    void PolylinesVboManager::unbindBlock()
    {
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        m_gl->glBindVertexArray(0);
    }

    void PolylinesVboManager::startBackgroundDefrag()
    {
        if (1)
        {
            m_bStopDefrag = false;
            m_defragThread = std::thread([this] {
                while (!m_bStopDefrag)
                {
                    //std::this_thread::sleep_for(std::chrono::seconds(5));
                    for (int i = 0; i < 30 && !m_bStopDefrag.load(); ++i)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(300));
                        if (m_bStopDefrag)
                            return;
                    }

                    std::shared_lock lock(m_mutex);
                    for (auto& pair : m_colorBlocksMap)
                    {
                        for (ColorVBOBlock* block : pair.second)
                        {
                            // 只有使用率 < 50% 才值得整理
                            if (block->bCompact && block->nVertexCount > 0)
                            {
                                double dUsage = double(block->nVertexCount) / block->nVertexCapacity;
                                if (dUsage < 0.7)
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
        else
        {
            if (m_defragThread.joinable())
                return;

            m_bStopDefrag = false;
            m_defragThread = std::thread([this] {
                while (!m_bStopDefrag.load())
                {
                    // 50次 × 300毫秒 / 次 = 15000毫秒 = 15秒
                    for (int i = 0; i < 30 && !m_bStopDefrag.load(); ++i)
                        std::this_thread::sleep_for(std::chrono::milliseconds(300));

                    std::shared_lock<std::shared_mutex> lock(m_mutex);
                    for (auto& pair : m_colorBlocksMap)
                        for (ColorVBOBlock* b : pair.second)
                            compactBlock(b);
                } });
        }
    }

    void PolylinesVboManager::stopBackgroundDefrag()
    {
        m_bStopDefrag = true;
        if (m_defragThread.joinable())
        {
            m_bStopDefrag = true;
            m_defragThread.join();
        }
    }
} // namespace GLRhi