#include "Render/LineBRenderer.h"
#include "Shader/PathBoldShader.h" 
#include <QDebug>

namespace GLRhi
{
    LineBRenderer::~LineBRenderer()
    {
        cleanup();
    }

    bool LineBRenderer::initialize(QOpenGLContext* context)
    {
        if (!context)
        {
            assert(false && "LineRenderer::initialize: context is null");
            return false;
        }

        m_context = context;
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if (!m_gl)
        {
            assert(false && "LineBRenderer initialize: OpenGL functions not available");
            return false;
        }
        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chPathBoldVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Geometry, chPathBoldGS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chPathBoldFS) ||
            !m_program->link())
        {
            deleteProgram(m_program);
            assert(false && "LineBRenderer: Shader link failed");
            return false;
        }

        m_program->bind();
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");
        m_uColorLoc = m_program->uniformLocation("uColor");
        m_uLineTypeLoc = m_program->uniformLocation("uLineType");
        m_uDashScaleLoc = m_program->uniformLocation("uDashScale");
        m_uThicknessLoc = m_program->uniformLocation("uThickness");
        m_uDepthLoc = m_program->uniformLocation("uDepth");

        bool bUniformError = (m_uCameraMatLoc < 0) || (m_uColorLoc < 0) || (m_uLineTypeLoc < 0) ||
            (m_uDashScaleLoc < 0) || (m_uThicknessLoc < 0) || (m_uDepthLoc < 0);

        if (bUniformError)
        {
            deleteProgram(m_program);
            assert(false && "LineBRenderer: Failed to get uniform locations");
            return false;
        }

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            deleteProgram(m_program);
            assert(false && "LineBRenderer: OpenGL error during initialization");
            return false;
        }

        m_program->release();
        return true;
    }

    void LineBRenderer::render(const float* matMVP)
    {
        if (!m_gl || !m_program)
            return;

        m_program->bind();
        if (m_uCameraMatLoc >= 0 && matMVP)
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix4x4(matMVP));

        for (auto& lineBInfo : m_lineBInfos)
        {
            if (lineBInfo.vao && lineBInfo.count > 0)
            {
                m_program->setUniformValue(m_uColorLoc,
                    QVector4D(lineBInfo.color.r(), lineBInfo.color.g(), lineBInfo.color.b(), lineBInfo.color.a()));

                m_program->setUniformValue(m_uLineTypeLoc, lineBInfo.lineType);
                m_program->setUniformValue(m_uDashScaleLoc, lineBInfo.dashScale);
                m_program->setUniformValue(m_uThicknessLoc, lineBInfo.thickness);
                m_program->setUniformValue(m_uDepthLoc, lineBInfo.color.d());

                m_gl->glBindVertexArray(lineBInfo.vao);
                m_gl->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineBInfo.count));
                m_gl->glBindVertexArray(0);
            }
        }

        m_program->release();
    }

    void LineBRenderer::cleanup()
    {
        if (!m_gl) return;

        for (auto& lineBInfo : m_lineBInfos)
        {
            deleteVaoVbo(lineBInfo.vao, lineBInfo.vbo);
            lineBInfo.verts.clear();
            lineBInfo.count = 0;
        }
        m_lineBInfos.clear();

        deleteProgram(m_program);

        m_gl = nullptr;
    }

    void LineBRenderer::updateData(float* data, size_t count, const Brush& color,
        int lineType, float dashScale, float thickness)
    {
        if (!m_gl || !data || count == 0)
            return;

        LineBInfoEx lineBInfo;
        lineBInfo.color = color;
        lineBInfo.lineType = lineType;
        lineBInfo.dashScale = dashScale;
        lineBInfo.thickness = thickness;

        size_t sz = count / 3;
        lineBInfo.verts.resize(sz);
        memcpy(lineBInfo.verts.data(), data, count * sizeof(float));
        lineBInfo.count = sz;

        m_gl->glGenVertexArrays(1, &lineBInfo.vao);
        m_gl->glGenBuffers(1, &lineBInfo.vbo);
        m_gl->glBindVertexArray(lineBInfo.vao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, lineBInfo.vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), lineBInfo.verts.data(), GL_STATIC_DRAW);
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        m_gl->glEnableVertexAttribArray(0);
        m_gl->glBindVertexArray(0);

        m_lineBInfos.push_back(lineBInfo);
    }
}