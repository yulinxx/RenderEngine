#include "Render/LineTestRenderer.h"
#include "Shader/PathShader.h"
#include "Shader/BaseLineShader.h"

#include <QDebug>
#include <cassert>

namespace GLRhi
{
    LineTestRenderer::LineTestRenderer()
    {
    }

    LineTestRenderer::~LineTestRenderer()
    {
        cleanup();
    }

    bool LineTestRenderer::initialize(QOpenGLContext* context)
    {
        if (!context)
        {
            assert(false && "LineTestRenderer::initialize: context is null");
            return false;
        }
        m_context = context;
        m_gl = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
        if (!m_gl)
        {
            assert(false && "LineTestRenderer::initialize: Failed to get OpenGL 3.3 Core functions");
            return false;
        }

        m_program = new QOpenGLShaderProgram;

        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, baseLineVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, baseLineFS) ||
            !m_program->link())
        {
            deleteProgram(m_program);
            assert(false && "LineTestRenderer: Shader link failed");
            return false;
        }

        m_uColorLoc = m_program->uniformLocation("uColor");
        m_uDepthLoc = m_program->uniformLocation("uDepth");
        m_uCameraMatLoc = m_program->uniformLocation("uCameraMat");

        bool bUniformError = (m_uColorLoc < 0) || (m_uDepthLoc < 0);
        if (bUniformError)
        {
            deleteProgram(m_program);
            assert(false && "LineTestRenderer: Failed to get uniform locations");
            return false;
        }

        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            cleanup();
            assert(false && "LineTestRenderer: OpenGL error during initialization");
            return false;
        }

        return true;
    }

    void LineTestRenderer::render(const float* matMVP)
    {
        if (!m_program || !m_gl || m_polylines.empty())
            return;

        // 如果数据发生变化，重新上传到GPU
        if (m_bDataDirty)
        {
            uploadDataToGPU();
            m_bDataDirty = false;
        }

        if (m_nVao == 0)
            return;

        m_program->bind();

        // 设置相机变换矩阵
        if (m_uCameraMatLoc >= 0 && matMVP)
        {
            QMatrix3x3 mat;
            mat(0, 0) = matMVP[0]; mat(1, 0) = matMVP[1]; mat(2, 0) = matMVP[2]; // 第一列
            mat(0, 1) = matMVP[3]; mat(1, 1) = matMVP[4]; mat(2, 1) = matMVP[5]; // 第二列
            mat(0, 2) = matMVP[6]; mat(1, 2) = matMVP[7]; mat(2, 2) = matMVP[8]; // 第三列
            m_program->setUniformValue(m_uCameraMatLoc, mat);
        }

        // 设置深度
        if (m_uDepthLoc >= 0)
        {
            m_program->setUniformValue(m_uDepthLoc, 0.0f);
        }

        m_gl->glBindVertexArray(m_nVao);
        // EBO is already bound in the VAO, no need to bind it separately
        
        // 禁用深度测试，确保线条总是可见
        // m_gl->glDepthMask(GL_FALSE);
        // m_gl->glDisable(GL_DEPTH_TEST);

        // 遍历所有折线并绘制
        GLuint indexOffset = 0;
        size_t drawCount = 0;
        for (const auto& pair : m_polylines)
        {
            const PolylineData& polyline = pair.second;
            GLuint vertexCount = static_cast<GLuint>(polyline.vertices.size() / 3);

            if (vertexCount < 2)
            {
                indexOffset += vertexCount;
                continue;
            }

            // 设置颜色
            if (m_uColorLoc >= 0)
            {
                m_program->setUniformValue(m_uColorLoc, QVector4D(polyline.r, polyline.g, polyline.b, 1.0f));
            }

            // 绘制折线
            void* byteOffset = reinterpret_cast<void*>(indexOffset * sizeof(GLuint));
            m_gl->glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(vertexCount), GL_UNSIGNED_INT, byteOffset);
            
            drawCount++;
            indexOffset += vertexCount;
        }
        
        if (drawCount > 0)
        {
            qDebug() << "[LineTestRenderer] render: Drew" << drawCount << "polylines";
        }
        
        // 恢复深度测试状态
        // m_gl->glEnable(GL_DEPTH_TEST);
        // m_gl->glDepthMask(GL_TRUE);

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void LineTestRenderer::uploadDataToGPU()
    {
        if (!m_gl)
        {
            qWarning() << "[LineTestRenderer] uploadDataToGPU: m_gl is null";
            return;
        }

        // 确保在正确的 OpenGL 上下文中
        if (QOpenGLContext::currentContext() != m_context)
        {
            qWarning() << "[LineTestRenderer] uploadDataToGPU: OpenGL context mismatch";
            return;
        }

        // 清理旧的buffer
        if (m_nVao != 0)
        {
            deleteVaoVbo(m_nVao, m_nVbo);
            deleteEbo(m_nEbo);
            m_nVao = 0;
            m_nVbo = 0;
            m_nEbo = 0;
        }

        if (m_polylines.empty())
        {
            qDebug() << "[LineTestRenderer] uploadDataToGPU: No polylines to upload";
            return;
        }

        // 合并所有折线的顶点数据
        std::vector<float> allVertices;
        std::vector<GLuint> allIndices;

        GLuint vertexOffset = 0;
        for (const auto& pair : m_polylines)
        {
            const PolylineData& polyline = pair.second;
            const auto& verts = polyline.vertices;

            if (verts.empty() || verts.size() % 3 != 0)
                continue;

            size_t vertexCount = verts.size() / 3;

            // 添加顶点数据
            allVertices.insert(allVertices.end(), verts.begin(), verts.end());

            // 添加索引数据（用于LINE_STRIP）
            for (size_t i = 0; i < vertexCount; ++i)
            {
                allIndices.push_back(vertexOffset + static_cast<GLuint>(i));
            }

            vertexOffset += static_cast<GLuint>(vertexCount);
        }

        if (allVertices.empty() || allIndices.empty())
        {
            qWarning() << "[LineTestRenderer] uploadDataToGPU: Empty vertex or index data";
            return;
        }

        qDebug() << "[LineTestRenderer] uploadDataToGPU: Uploading" 
                 << m_polylines.size() << "polylines,"
                 << (allVertices.size() / 3) << "vertices,"
                 << allIndices.size() << "indices";

        // 创建VAO
        m_gl->glGenVertexArrays(1, &m_nVao);
        if (m_nVao == 0)
        {
            qWarning() << "[LineTestRenderer] uploadDataToGPU: Failed to create VAO";
            return;
        }
        m_gl->glBindVertexArray(m_nVao);

        // 创建VBO并上传顶点数据
        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, 
                          static_cast<GLsizeiptr>(allVertices.size() * sizeof(float)), 
                          allVertices.data(), 
                          GL_DYNAMIC_DRAW);

        // 设置顶点属性指针
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        m_gl->glEnableVertexAttribArray(0);

        // 创建EBO并上传索引数据
        m_gl->glGenBuffers(1, &m_nEbo);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                          static_cast<GLsizeiptr>(allIndices.size() * sizeof(GLuint)), 
                          allIndices.data(), 
                          GL_DYNAMIC_DRAW);

        unbindABE();

        // 检查 OpenGL 错误
        GLenum error = m_gl->glGetError();
        if (error != GL_NO_ERROR)
        {
            qWarning() << "[LineTestRenderer] uploadDataToGPU: OpenGL error" << error;
        }
        else
        {
            qDebug() << "[LineTestRenderer] uploadDataToGPU: Successfully uploaded data to GPU";
        }
    }

    void LineTestRenderer::cleanup()
    {
        if (!m_gl)
            return;

        unbindABE();
        deleteVaoVbo(m_nVao, m_nVbo);
        deleteEbo(m_nEbo);
        deleteProgram(m_program);

        m_polylines.clear();
        m_bDataDirty = true;
        m_gl = nullptr;
    }

    void LineTestRenderer::clearData()
    {
        m_polylines.clear();
        m_bDataDirty = true;

        if (m_gl && m_nVao != 0)
        {
            deleteVaoVbo(m_nVao, m_nVbo);
            deleteEbo(m_nEbo);
        }
    }

    void LineTestRenderer::addPolyline(long long id, const float* verts, size_t n, float r, float g, float b)
    {
        if (!verts || n < 6 || n % 3 != 0)
        {
            qWarning() << "[LineTestRenderer] addPolyline: Invalid parameters. id=" << id << "n=" << n;
            return;
        }

        PolylineData polyline;
        polyline.id = id;
        polyline.vertices.assign(verts, verts + n);
        polyline.r = r;
        polyline.g = g;
        polyline.b = b;

        m_polylines[id] = std::move(polyline);
        m_bDataDirty = true;

        qDebug() << "[LineTestRenderer] Added polyline: id=" << id 
                 << "vertices=" << (n / 3) 
                 << "color=(" << r << "," << g << "," << b << ")"
                 << "total polylines=" << m_polylines.size();
    }

}