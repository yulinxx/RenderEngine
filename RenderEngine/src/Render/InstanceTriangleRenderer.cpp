#include "Render/InstanceTriangleRenderer.h"
#include "Shader/InstanceTriangleShader.h"
#include <QDebug>
#include <cassert>

namespace GLRhi
{
    InstanceTriangleRenderer::~InstanceTriangleRenderer()
    {
        cleanup();
    }

    bool InstanceTriangleRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            assert(false && "InstanceTriangleRenderer::initialize: OpenGL functions not available");
            return false;
        }

        // 创建着色器程序
        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, instanceTriangleVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, instanceTriangleFS) ||
            !m_program->link())
        {
            cleanup();
            assert(false && "InstanceTriangleRenderer: Shader link failed");
            return false;
        }

        // 获取uniform位置
        m_program->bind();
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");

        bool bUniformError = (m_uCameraMatLoc < 0);
        if (bUniformError)
        {
            cleanup();
            assert(false && "InstanceTriangleRenderer: Failed to get uniform locations");
            return false;
        }
        m_program->release();

        // 创建VAO和VBO
        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glGenBuffers(1, &m_nVertexVbo);
        m_gl->glGenBuffers(1, &m_nInstanceVbo);

        // 基础三角形顶点数据（等边三角形，中心在原点）
        float baseTriangleVertices[] = {
            -1.0f, -0.577f, 0.0f, // 左下顶点
             1.0f, -0.577f, 0.0f, // 右下顶点
             0.0f,  1.155f, 0.0f  // 顶部顶点
        };

        m_gl->glBindVertexArray(m_nVao);

        // 配置顶点VBO
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVertexVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, sizeof(baseTriangleVertices), baseTriangleVertices, GL_STATIC_DRAW);

        // 顶点位置属性
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        // 配置实例化数据VBO（暂时不填充数据）
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nInstanceVbo);

        // 实例属性：三角形第一个顶点
        m_gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceTriangleData), (void*)offsetof(InstanceTriangleData, pos1));
        m_gl->glEnableVertexAttribArray(1);
        m_gl->glVertexAttribDivisor(1, 1); // 每个实例更新一次

        // 实例属性：三角形第二个顶点
        m_gl->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceTriangleData), (void*)offsetof(InstanceTriangleData, pos2));
        m_gl->glEnableVertexAttribArray(2);
        m_gl->glVertexAttribDivisor(2, 1);

        // 实例属性：三角形第三个顶点
        m_gl->glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceTriangleData), (void*)offsetof(InstanceTriangleData, pos3));
        m_gl->glEnableVertexAttribArray(3);
        m_gl->glVertexAttribDivisor(3, 1);

        // 实例属性：三角形颜色
        m_gl->glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceTriangleData), (void*)offsetof(InstanceTriangleData, color));
        m_gl->glEnableVertexAttribArray(4);
        m_gl->glVertexAttribDivisor(4, 1);

        // 实例属性：深度值
        m_gl->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceTriangleData), (void*)offsetof(InstanceTriangleData, depth));
        m_gl->glEnableVertexAttribArray(5);
        m_gl->glVertexAttribDivisor(5, 1);

        // 检查OpenGL错误
        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "InstanceTriangleRenderer: OpenGL error during initialization");
            return false;
        }

        m_gl->glBindVertexArray(0);

        return true;
    }

    void InstanceTriangleRenderer::updateInstances(const std::vector<InstanceTriangleData>& vData)
    {
        if (!m_gl || !m_nVao || vData.empty())
            return;

        m_nInstanceCount = vData.size();

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nInstanceVbo);

        // 获取当前缓冲区大小
        GLint bufferSize = 0;
        m_gl->glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

        const GLsizeiptr newSize = static_cast<GLsizeiptr>(vData.size() * sizeof(InstanceTriangleData));

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

    void InstanceTriangleRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || !m_nVao || m_nInstanceCount == 0)
            return;

        m_program->bind();
        m_gl->glBindVertexArray(m_nVao);
        m_gl->glEnable(GL_DEPTH_TEST);

        if (m_bBlend)
        {
            m_gl->glEnable(GL_BLEND);
            m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            m_gl->glBlendEquation(GL_FUNC_ADD);
        }
        else
        {
            m_gl->glDisable(GL_BLEND);
        }

        // 设置相机矩阵
        if (m_uCameraMatLoc >= 0)
        {
            float identity[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
            QMatrix4x4 mat(cameraMat);
            m_program->setUniformValue(m_uCameraMatLoc, mat);
        }

        // 使用实例化绘制
        m_gl->glDrawArraysInstanced(GL_TRIANGLES, 0, 3, static_cast<GLsizei>(m_nInstanceCount));

        // 仅在启用了混合的情况下才禁用
        if (m_bBlend)
        {
            m_gl->glDisable(GL_BLEND);
        }

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void InstanceTriangleRenderer::cleanup()
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

    void InstanceTriangleRenderer::setBlendEnabled(bool enabled)
    {
        m_bBlend = enabled;
    }

    bool InstanceTriangleRenderer::isBlendEnabled() const
    {
        return m_bBlend;
    }
}