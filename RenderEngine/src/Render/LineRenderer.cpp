#include "Render/LineRenderer.h"
#include "Shader/PathShader.h"
#include "Shader/BaseLineShader.h"
#include <QDebug>
#include <cassert>

namespace GLRhi
{
    LineRenderer::LineRenderer()
    {
        m_mat.setToIdentity();
    }

    LineRenderer::~LineRenderer()
    {
        cleanup();
    }

    bool LineRenderer::initialize(QOpenGLContext* context)
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
            assert(false && "LineRenderer::initialize: Failed to get OpenGL 3.3 Core functions");
            return false;
        }

        if (!m_lineBuffer.initialize(context))
        {
            assert(false && "LineRenderer::initialize: Failed to initialize LineBufferManager");
            return false;
        }

        m_program = new QOpenGLShaderProgram;

        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, baseLineVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, baseLineFS) ||
            !m_program->link())
        {
            deleteProgram(m_program);
            assert(false && "LineRenderer: Shader link failed");
            return false;
        }

        m_uColorLoc = m_program->uniformLocation("uColor");
        m_uDepthLoc = m_program->uniformLocation("uDepth");
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");

        bool bUniformError = (m_uColorLoc < 0) || (m_uDepthLoc < 0);
        if (bUniformError)
        {
            deleteProgram(m_program);
            assert(false && "LineRenderer: Failed to get uniform locations");
            return false;
        }

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "LineRenderer: OpenGL error during initialization");
            return false;
        }

        return true;
    }

    void LineRenderer::render(const float* matMVP)
    {
        if (!m_program)
            return;

        m_program->bind();

        m_program->bind();

        if (m_uCameraMatLoc >= 0)
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix4x4(matMVP));

        m_lineBuffer.renderVisiblePrimitivesEx();
        m_program->release();

        if (0)
        {

            if (m_uCameraMatLoc >= 0 && matMVP)
            {
                // 第一列
                m_mat(0, 0) = matMVP[0]; m_mat(1, 0) = matMVP[1]; m_mat(2, 0) = matMVP[2];
                // 第二列
                m_mat(0, 1) = matMVP[3]; m_mat(1, 1) = matMVP[4]; m_mat(2, 1) = matMVP[5];
                // 第三列
                m_mat(0, 2) = matMVP[6]; m_mat(1, 2) = matMVP[7]; m_mat(2, 2) = matMVP[8];

                m_program->setUniformValue(m_uCameraMatLoc, m_mat);
            }

            // 设置深度
            if (m_uDepthLoc >= 0)
            {
                m_program->setUniformValue(m_uDepthLoc, 0.0f);
            }

            m_lineBuffer.renderVisiblePrimitivesEx();
        }
        m_program->release();

        // qDebug() << "[LineRenderer] render() completed";

        return;
    }

    void LineRenderer::cleanup()
    {
        clearData();
        //m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        //if (!m_gl)
        //    return;

        //unbindABE();

        //m_gl = nullptr;
    }

    void LineRenderer::clearData()
    {
        m_lineBuffer.clearAllPrimitives();
    }

    void LineRenderer::updateData(const std::vector<PolylineData>& vPolylineDatas)
    {
        //addPolylines(std::vector<PolylineData>& vPlDatas)
        m_lineBuffer.addPolylines(vPolylineDatas);
        return;
    }
}