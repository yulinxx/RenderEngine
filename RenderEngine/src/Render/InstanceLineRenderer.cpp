#include "Render/InstanceLineRenderer.h"
#include "Shader/InstanceLineShader.h"
#include <QDebug>
#include <cassert>

namespace GLRhi
{
    InstanceLineRenderer::~InstanceLineRenderer()
    {
        cleanup();
    }

    bool InstanceLineRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(false && "InstanceLineRenderer::initialize: OpenGL functions not available");
            return false;
        }

        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, instanceLineVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, instanceLineFS) ||
            !m_program->link())
        {
            cleanup();
            assert(false && "InstanceLineRenderer: Shader link failed");
            return false;
        }

        m_program->bind();
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");

        bool bUniformError = (m_uCameraMatLoc < 0);
        if (bUniformError)
        {
            cleanup();
            assert(false && "InstanceLineRenderer: Failed to get uniform locations");
            return false;
        }

        m_program->release();

        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glGenBuffers(1, &m_nVertexVbo);
        m_gl->glGenBuffers(1, &m_nInstanceVbo);

        // 基础线段顶点数据 - 使用四个顶点表示线段的上下两侧
        // 这样可以通过偏移实现不同宽度的线段
        float baseLineVertices[] = {
            -1.0f, -1.0f, 0.0f, // 第一个端点的下侧
            -1.0f, 1.0f, 0.0f,  // 第一个端点的上侧
            1.0f, -1.0f, 0.0f,  // 第二个端点的下侧
            1.0f, 1.0f, 0.0f    // 第二个端点的上侧
        };

        m_gl->glBindVertexArray(m_nVao);

        // 配置顶点VBO
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVertexVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, sizeof(baseLineVertices), baseLineVertices, GL_STATIC_DRAW);

        // 顶点位置属性
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        // 配置实例化数据VBO（暂时不填充数据）
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nInstanceVbo);

        // 实例属性：线段第一个端点
        m_gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceLineData), (void*)offsetof(InstanceLineData, pos1));
        m_gl->glEnableVertexAttribArray(1);
        m_gl->glVertexAttribDivisor(1, 1); // 每个实例更新一次

        // 实例属性：线段第二个端点
        m_gl->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceLineData), (void*)offsetof(InstanceLineData, pos2));
        m_gl->glEnableVertexAttribArray(2);
        m_gl->glVertexAttribDivisor(2, 1);

        // 实例属性：线段颜色
        m_gl->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceLineData), (void*)offsetof(InstanceLineData, color));
        m_gl->glEnableVertexAttribArray(3);
        m_gl->glVertexAttribDivisor(3, 1);

        // 实例属性：线段宽度
        m_gl->glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceLineData), (void*)offsetof(InstanceLineData, width));
        m_gl->glEnableVertexAttribArray(4);
        m_gl->glVertexAttribDivisor(4, 1);

        // 实例属性：深度值
        m_gl->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceLineData), (void*)offsetof(InstanceLineData, depth));
        m_gl->glEnableVertexAttribArray(5);
        m_gl->glVertexAttribDivisor(5, 1);

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "InstanceLineRenderer: OpenGL error during initialization");
            return false;
        }

        m_gl->glBindVertexArray(0);

        return true;
    }

    void InstanceLineRenderer::updateInstances(const std::vector<InstanceLineData>& vData)
    {
        if (!m_gl || !m_nVao || vData.empty())
            return;

        m_nInstanceCount = vData.size();

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nInstanceVbo);

        // 获取当前缓冲区大小
        GLint bufferSize = 0;
        m_gl->glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

        const GLsizeiptr newSize = static_cast<GLsizeiptr>(vData.size() * sizeof(InstanceLineData));

        // 如果需要更大的缓冲区或者缓冲区未初始化，则重新分配
        if (newSize > bufferSize)
        {
            m_gl->glBufferData(GL_ARRAY_BUFFER, newSize, vData.data(), GL_DYNAMIC_DRAW);
        }
        else
        {
            // 否则只更新数据
            m_gl->glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, vData.data());
        }

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void InstanceLineRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || !m_nVao || m_nInstanceCount == 0)
            return;

        m_program->bind();
        m_gl->glBindVertexArray(m_nVao);

        // 设置相机矩阵
        if (m_uCameraMatLoc >= 0)
        {
            //float identity[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix4x4(cameraMat));
        }

        // 移除了不再需要的线宽uniform设置
        // 现在使用顶点着色器中的aInstanceWidth属性来控制线段宽度

        // 启用混合
        m_gl->glEnable(GL_BLEND);
        m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 使用实例化绘制，使用三角形带模式渲染线段
        m_gl->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(m_nInstanceCount));

        // 禁用混合和线段平滑
        m_gl->glDisable(GL_BLEND);
        m_gl->glDisable(GL_LINE_SMOOTH);

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void InstanceLineRenderer::cleanup()
    {
        if (!m_gl)
            return;

        unbindABE();

        // 清理VAO和VBO
        if (m_nVao != 0)
        {
            m_gl->glDeleteVertexArrays(1, &m_nVao);
            m_nVao = 0;
        }

        if (m_nVertexVbo != 0)
        {
            m_gl->glDeleteBuffers(1, &m_nVertexVbo);
            m_nVertexVbo = 0;
        }

        if (m_nInstanceVbo != 0)
        {
            m_gl->glDeleteBuffers(1, &m_nInstanceVbo);
            m_nInstanceVbo = 0;
        }

        // 清理程序
        deleteProgram(m_program);

        // 重置计数器
        m_nInstanceCount = 0;

        m_gl = nullptr;
    }
}