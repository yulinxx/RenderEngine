#include "Render/TriangleRenderer.h"
#include "Shader/BaseTriangleShader.h"
#include <QDebug>
#include <cassert>

namespace GLRhi
{
    bool TriangleRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            qWarning() << "TriangleRenderer: OpenGL functions not available";
            return false;
        }

        // 创建着色器程序
        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, baseTriangleVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, baseTriangleFS) ||
            !m_program->link())
        {
            qWarning() << "Triangle shader link failed:" << m_program->log();
            cleanup();
            return false;
        }

        // 获取Uniform位置
        m_program->bind();
        m_colorLoc = m_program->uniformLocation("color");
        m_depthLoc = m_program->uniformLocation("depth");
        m_cameraMatLoc = m_program->uniformLocation("cameraMat");
        m_program->release();

        // 创建并配置VAO/VBO/EBO
        m_gl->glGenVertexArrays(1, &m_vao);
        m_gl->glGenBuffers(1, &m_vbo);
        m_gl->glGenBuffers(1, &m_ebo);

        m_gl->glBindVertexArray(m_vao);

        // 配置顶点属性：位置 (x, y, len)
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

        m_gl->glBindVertexArray(0);

        return true;
    }

    void TriangleRenderer::updateData(std::vector<TriangleData>& vTriDatas)
    {
        if (!m_gl || !m_vao)
            return;

        m_vTriDatas = std::move(vTriDatas); // 保留数据副本

        // 计算总大小
        size_t totalVertices = 0;
        size_t totalIndices = 0;
        m_batches.clear();
        m_batches.reserve(m_vTriDatas.size());

        // 第一遍：计算总大小和每个批次的索引范围
        for (const auto& triData : m_vTriDatas)
        {
            Batch batch;
            batch.indexOffset = totalIndices;  // 存储索引的个数偏移，不是字节偏移！
            batch.indexCount = triData.indices.size();
            batch.brush = triData.brush;
            m_batches.push_back(batch);

            totalVertices += triData.vertices.size() / 3;
            totalIndices += triData.indices.size();
        }

        // 合并所有顶点数据
        std::vector<float> allVertices;
        allVertices.reserve(totalVertices * 3);
        for (const auto& triData : m_vTriDatas)
        {
            allVertices.insert(allVertices.end(), triData.vertices.begin(), triData.vertices.end());
        }

        // 合并所有索引数据（需要重新计算顶点偏移）
        std::vector<unsigned int> allIndices;
        allIndices.reserve(totalIndices);

        unsigned int vertexOffset = 0;
        for (const auto& triData : m_vTriDatas)
        {
            for (unsigned int index : triData.indices)
            {
                allIndices.push_back(index + vertexOffset);
            }
            vertexOffset += triData.vertices.size() / 3;
        }

        // 更新GPU缓冲
        m_gl->glBindVertexArray(m_vao);

        // 更新顶点缓冲
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
            allVertices.data(), GL_STATIC_DRAW);

        // 更新索引缓冲
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(unsigned int),
            allIndices.data(), GL_STATIC_DRAW);

        m_gl->glBindVertexArray(0);
    }

    void TriangleRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || m_batches.empty()) return;

        m_program->bind();
        m_gl->glBindVertexArray(m_vao);

        // 启用必要的 OpenGL 状态
        m_gl->glEnable(GL_DEPTH_TEST);
        m_gl->glEnable(GL_BLEND);
        m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 设置相机矩阵
        float identity[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
        if (m_cameraMatLoc >= 0)
        {
            QMatrix4x4 mat(identity);
            m_program->setUniformValue(m_cameraMatLoc, mat);
        }
        // if (m_cameraMatLoc >= 0)
        // {
        //     QMatrix4x4 mat;
        //     if (cameraMat)
        //     {
        //         mat = QMatrix4x4(cameraMat).transposed();
        //     }
        //     else
        //     {
        //         mat.setToIdentity();
        //     }
        //     m_program->setUniformValue(m_cameraMatLoc, mat);
        // }

        // 遍历每个批次并绘制
        for (const auto& batch : m_batches)
        {
            const auto& b = batch.brush;
            m_program->setUniformValue(m_depthLoc, b.d());
            m_program->setUniformValue(m_colorLoc, QVector4D(b.r(), b.g(), b.b(), b.a()));

            // 绘制该批次 - 关键修正：将索引偏移转换为字节偏移
            m_gl->glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT,
                (void*)(batch.indexOffset * sizeof(unsigned int)));
        }

        m_gl->glBindVertexArray(0);
        m_program->release();
    }
    void TriangleRenderer::cleanup()
    {
        if (!m_gl) return;

        if (m_vao)
        {
            m_gl->glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_vbo)
        {
            m_gl->glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_ebo)
        {
            m_gl->glDeleteBuffers(1, &m_ebo);
            m_ebo = 0;
        }

        delete m_program;
        m_program = nullptr;
        m_gl = nullptr;
    }
}