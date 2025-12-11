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

    bool InstanceLineRenderer::initialize(QOpenGLContext* context)
    {
        if (!context)
        {
            assert(false && "InstanceLineRenderer::initialize: context is null");
            return false;
        }
        m_context = context;
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
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

        // 线段数据
        float baseLineVertices[] = {
            -1.0f, -1.0f, 0.0f, // 第一个点的下侧
            -1.0f, 1.0f, 0.0f,  // 第一个点的上侧
            1.0f, -1.0f, 0.0f,  // 第二个点的下侧
            1.0f, 1.0f, 0.0f    // 第二个点的上侧
        };

        m_gl->glBindVertexArray(m_nVao);

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVertexVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, sizeof(baseLineVertices), baseLineVertices, GL_STATIC_DRAW);

        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nInstanceVbo);

        m_gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceLineData), (void*)offsetof(InstanceLineData, pos1));
        m_gl->glEnableVertexAttribArray(1);
        m_gl->glVertexAttribDivisor(1, 1);

        m_gl->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceLineData), (void*)offsetof(InstanceLineData, pos2));
        m_gl->glEnableVertexAttribArray(2);
        m_gl->glVertexAttribDivisor(2, 1);

        m_gl->glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceLineData), (void*)offsetof(InstanceLineData, color));
        m_gl->glEnableVertexAttribArray(3);
        m_gl->glVertexAttribDivisor(3, 1);

        m_gl->glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceLineData), (void*)offsetof(InstanceLineData, width));
        m_gl->glEnableVertexAttribArray(4);
        m_gl->glVertexAttribDivisor(4, 1);

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

        // 缓冲区大小
        GLint nBfferSize = 0;
        m_gl->glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &nBfferSize);

        const GLsizeiptr newSize = static_cast<GLsizeiptr>(vData.size() * sizeof(InstanceLineData));

        if (newSize > nBfferSize)
            m_gl->glBufferData(GL_ARRAY_BUFFER, newSize, vData.data(), GL_DYNAMIC_DRAW);
        else
            m_gl->glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, vData.data());

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void InstanceLineRenderer::render(const float* matMVP)
    {
        if (!m_gl || !m_program || !m_nVao || m_nInstanceCount == 0)
            return;

        m_program->bind();
        m_gl->glBindVertexArray(m_nVao);

        // 设置相机矩阵
        if (m_uCameraMatLoc >= 0 && matMVP)
        {
            //float identity[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix4x4(matMVP));
        }

        m_gl->glEnable(GL_BLEND);
        m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 实例化绘制
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

        deleteProgram(m_program);

        m_nInstanceCount = 0;
        m_gl = nullptr;
    }
}