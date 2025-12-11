#include "Render/InstanceTextureRenderer.h"
#include "Shader/InstanceTextureShader.h"
#include <QDebug>

namespace GLRhi
{
    InstanceTextureRenderer::~InstanceTextureRenderer()
    {
        cleanup();
    }

    bool InstanceTextureRenderer::initialize(QOpenGLContext* context)
    {
        if (!context)
        {
            assert(false && "InstanceTextureRenderer::initialize: context is null");
            return false;
        }
        m_context = context;
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if (!m_gl)
        {
            assert(false && "InstanceTextureRenderer::initialize: OpenGL functions not available");
            return false;
        }
        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, chInstanceVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, chInstanceFS) ||
            !m_program->link())
        {
            cleanup();
            assert(false && "InstanceTextureRenderer: Shader link failed");
            return false;
        }

        m_program->bind();
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");
        m_uTexArrayLoc = m_program->uniformLocation("uTexArray");

        bool bUniformError = (m_uCameraMatLoc < 0) || (m_uTexArrayLoc < 0);
        if (bUniformError)
        {
            cleanup();
            assert(false && "InstanceTextureRenderer: Failed to get uniform locations");
            return false;
        }
        m_program->release();

        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glGenBuffers(1, &m_nVbo);

        float arrUnitQuad[] = {
            // 位置        纹理坐标
            -0.5f, -0.5f, 0.0f, 0.0f, // 左下
            0.5f, -0.5f, 1.0f, 0.0f,  // 右下
            0.5f, 0.5f, 1.0f, 1.0f,   // 右上
            -0.5f, 0.5f, 0.0f, 1.0f   // 左上
        };

        m_gl->glGenBuffers(1, &m_nQuadVbo);

        if (m_nQuadVbo == 0)
        {
            cleanup();
            assert(false && "InstanceTextureRenderer: Failed to create quad VBO");
            return false;
        }

        m_gl->glBindVertexArray(m_nVao);

        // m_nQuadVbo
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nQuadVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, sizeof(arrUnitQuad), arrUnitQuad, GL_STATIC_DRAW);

        // 位置
        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        // 纹理
        m_gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(1);

        // m_nVbo
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);

        // 实例位置
        m_gl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceTexData), (void*)0);
        m_gl->glEnableVertexAttribArray(2);
        m_gl->glVertexAttribDivisor(2, 1);

        // 尺寸
        m_gl->glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(InstanceTexData), (void*)(2 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(3);
        m_gl->glVertexAttribDivisor(3, 1);

        // 纹理
        m_gl->glVertexAttribIPointer(4, 1, GL_INT, sizeof(InstanceTexData), (void*)(4 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(4);
        m_gl->glVertexAttribDivisor(4, 1);

        // 透明度
        m_gl->glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceTexData), (void*)(5 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(5);
        m_gl->glVertexAttribDivisor(5, 1);

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "InstanceTextureRenderer: OpenGL error during initialization");
            return false;
        }

        m_gl->glBindVertexArray(0);

        return true;
    }

    void InstanceTextureRenderer::setTextureArray(GLuint texArrayId, int layerCount)
    {
        m_texArray = texArrayId;
        m_layerCount = layerCount;
    }

    void InstanceTextureRenderer::updateInstances(const std::vector<InstanceTexData>& vData)
    {
        if (!m_gl || !m_nVao)
            return;

        m_nInstCount = vData.size();
        if (m_nInstCount == 0)
            return;

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);

        GLint bufferSize = 0;
        m_gl->glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

        const GLsizeiptr nNewSz = static_cast<GLsizeiptr>(vData.size() * sizeof(InstanceTexData));

        if (nNewSz > bufferSize)
        {
            m_gl->glBufferData(GL_ARRAY_BUFFER,
                nNewSz,
                vData.data(),
                GL_DYNAMIC_DRAW);
        }
        else
        {
            m_gl->glBufferSubData(GL_ARRAY_BUFFER, 0, nNewSz, vData.data());
        }

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void InstanceTextureRenderer::render(const float* matMVP)
    {
        if (!m_gl || !m_program || m_nInstCount == 0 || m_texArray == 0)
            return;

        m_program->bind();
        m_gl->glBindVertexArray(m_nVao);

        //m_gl->glEnable(GL_BLEND);
        //m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // if (m_cameraMatLoc >= 0)
        //{
        //     QMatrix4x4 mat = cameraMat ? QMatrix4x4(cameraMat).transposed()
        //         : QMatrix4x4();
        //     m_program->setUniformValue(m_cameraMatLoc, mat);
        // }

        if (m_uCameraMatLoc >= 0 && matMVP)
            m_program->setUniformValue(m_uCameraMatLoc, QMatrix4x4(matMVP));

        if (m_uTexArrayLoc >= 0)
        {
            m_gl->glActiveTexture(GL_TEXTURE0);
            m_gl->glBindTexture(GL_TEXTURE_2D_ARRAY, m_texArray);
            m_program->setUniformValue(m_uTexArrayLoc, 0);
        }

        m_gl->glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, static_cast<GLsizei>(m_nInstCount));

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void InstanceTextureRenderer::cleanup()
    {
        if (!m_gl)
            return;

        deleteVaoVbo(m_nVao, m_nVbo);
        deleteVbo(m_nQuadVbo);
        deleteProgram(m_program);

        m_texArray = 0;
        m_nInstCount = 0;
        m_layerCount = 0;

        m_gl = nullptr;
    }
}