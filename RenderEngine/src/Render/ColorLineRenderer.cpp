#include "ColorLineRenderer.h"
#include "Shader/PathShader.h"
#include "Shader/BaseLineShader.h"
#include <QDebug>

namespace GLRhi
{
    bool ColorLineRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(!"ColorLineRenderer::initialize: OpenGL functions not available");
            return false;
        }

        m_program = new QOpenGLShaderProgram;

        // 加载顶点着色器和片段着色器
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, baseLineVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, baseLineFS) ||
            !m_program->link())
        {
            qWarning() << "Line shader link failed:" << m_program->log();
            deleteProgram(m_program);
            return false;
        }

        // 获取Uniform变量位置
        //m_program->bind();
        //m_cameraMatLoc = m_program->uniformLocation("cameraMat");
        //m_program->release();
        return true;
    }

    void ColorLineRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || m_vIndices.empty() || m_vPlineInfos.empty())
            return;

        m_program->bind();

        // 设置相机矩阵
        //if (m_cameraMatLoc >= 0)
        //{
        //    //m_program->setUniformValue(m_cameraMatLoc, QMatrix4x4(cameraMat));
        //}

        // 启用图元重启
        m_gl->glEnable(GL_PRIMITIVE_RESTART);
        m_gl->glPrimitiveRestartIndex(0xFFFFFFFF);

        // 绑定VAO
        m_gl->glBindVertexArray(m_nVao);
        size_t indexOffset = 0;
        const size_t maxIndices = m_vIndices.size();

        // 确保m_vPlineInfos和m_vIndices之间的同步
        // 限制处理的多段线数量，避免处理不存在对应索引数据的多段线
        size_t polylineCount = 0;
        const size_t maxPolylineCount = m_vPlineInfos.size();

        while (polylineCount < maxPolylineCount && indexOffset < maxIndices)
        {
            const auto& polyline = m_vPlineInfos[polylineCount];
            m_program->setUniformValue("uColor",
                QVector4D(polyline.brush.r(), polyline.brush.g(), polyline.brush.b(), polyline.brush.a()));
            m_program->setUniformValue("uDepth", polyline.brush.d());

            // 查找当前多段线的索引数量（从当前偏移位置开始，直到找到图元重启索引）
            size_t indexCount = 0;
            size_t maxIterations = maxIndices - indexOffset; // 避免无限循环

            while (indexCount < maxIterations &&
                indexOffset + indexCount < maxIndices &&
                m_vIndices[indexOffset + indexCount] != 0xFFFFFFFF)
            {
                indexCount++;
            }

            // 如果找到了图元重启索引，则索引数量包含当前顶点数和重启索引
            if (indexOffset + indexCount < maxIndices && m_vIndices[indexOffset + indexCount] == 0xFFFFFFFF)
            {
                indexCount += 1; // 包含图元重启索引
            }

            // 只有当有有效的索引时才绘制
            if (indexCount > 0 && indexOffset + indexCount <= maxIndices)
            {
                m_gl->glDrawElements(GL_LINE_STRIP,
                    static_cast<GLsizei>(indexCount),
                    GL_UNSIGNED_INT,
                    (void*)(indexOffset * sizeof(GLuint)));
                indexOffset += indexCount;
            }

            polylineCount++;
        }

        // 禁用图元重启
        m_gl->glDisable(GL_PRIMITIVE_RESTART);

        // 解绑VAO
        m_gl->glBindVertexArray(0);

        m_program->release();
    }

    void ColorLineRenderer::cleanup()
    {
        if (!m_gl)
            return;

        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        deleteVaoVbo(m_nVao, m_nVbo);
        if (m_nEbo)
        {
            m_gl->glDeleteBuffers(1, &m_nEbo);
            m_nEbo = 0;
        }

        deleteProgram(m_program);
        m_gl = nullptr;
    }

    void ColorLineRenderer::updateData(float* data, size_t count, const Brush& b)
    {
        if (!m_gl || !data || count == 0) return;

        PolylineData line;
        line.brush = b;
        line.vertices.assign(data, data + count * 3); // 每个顶点3个float（x,y,depth）

        //// 创建VAO/VBO
        //m_gl->glGenVertexArrays(1, &line.vao);
        //m_gl->glGenBuffers(1, &line.m_nVbo);
        //m_gl->glBindVertexArray(line.vao);
        //m_gl->glBindBuffer(GL_ARRAY_BUFFER, line.m_nVbo);
        //m_gl->glBufferData(GL_ARRAY_BUFFER, line.vertices.size() * sizeof(float),
        //    line.vertices.data(), GL_STATIC_DRAW);

        //// 顶点属性：位置（x,y）+ 深度（z）
        //m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        //m_gl->glEnableVertexAttribArray(0);
        //m_gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        //    (void*)(2 * sizeof(float)));
        //m_gl->glEnableVertexAttribArray(1);

        //m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        //m_gl->glBindVertexArray(0);

        //m_vPlineInfos.push_back(std::move(line));
    }

    // 准备缓冲区
    void ColorLineRenderer::prepareBuffers(const std::vector<PolylineData>& polylines)
    {
        if (!m_gl || polylines.empty())
            return;

        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // 2. 安全删除旧资源（如果存在）
        deleteVaoVbo(m_nVao, m_nVbo);

        if (m_nEbo)
        {
            m_gl->glDeleteBuffers(1, &m_nEbo);
            m_nEbo = 0;
        }

        // 3. 清空数据
        m_vPlineInfos.clear();
        m_vIndices.clear();

        // 4. 准备新数据（后续逻辑保持不变）
        std::vector<float> vVertices;
        size_t nVertexOffset = 0;

        for (const auto& polyline : polylines)
        {
            size_t vertexSz = polyline.vertices.size();
            if (vertexSz % 3 != 0 || vertexSz == 0) // 增加空检查
            {
                qWarning() << "Invalid vertex data: size=" << vertexSz;
                continue;
            }

            m_vPlineInfos.push_back(polyline); // 直接拷贝有用数据

            size_t vertexCount = vertexSz / 3;
            vVertices.insert(vVertices.end(), polyline.vertices.begin(), polyline.vertices.end());

            // 生成索引
            for (size_t i = 0; i < vertexCount; ++i)
            {
                m_vIndices.push_back(static_cast<GLuint>(nVertexOffset + i));
            }
            m_vIndices.push_back(0xFFFFFFFF); // 图元重启索引
            nVertexOffset += vertexCount;
        }

        // 5. 创建新缓冲区（后续逻辑保持不变）
        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glBindVertexArray(m_nVao);

        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, vVertices.size() * sizeof(float), vVertices.data(), GL_STATIC_DRAW);

        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        m_gl->glGenBuffers(1, &m_nEbo);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vIndices.size() * sizeof(GLuint), m_vIndices.data(), GL_STATIC_DRAW);

        // 6. 解绑
        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        // EBO绑定到VAO，不需要单独解绑
    }
}