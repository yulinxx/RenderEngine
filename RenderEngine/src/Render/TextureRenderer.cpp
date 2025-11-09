// TextureRenderer.cpp
#include "Render/TextureRenderer.h"
#include <QDebug>

namespace GLRhi
{
    const char* textureVS = R"(
        #version 330 core
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 texCoord;

        out vec2 v_TexCoord;
        uniform mat4 cameraMat;

        void main() {
            gl_Position = cameraMat * vec4(position, 0.0, 1.0);
            v_TexCoord = texCoord;
        }
    )";

    const char* textureFS = R"(
        #version 330 core
        in vec2 v_TexCoord;
        out vec4 fragColor;

        uniform sampler2D tex;
        uniform float alpha;

        void main() {
            vec4 color = texture(tex, v_TexCoord);
            fragColor = vec4(color.rgb, color.a * alpha);
        }
    )";

    bool TextureRenderer::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
            return false;

        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, textureVS) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, textureFS) ||
            !m_program->link())
        {
            qWarning() << "Texture shader link failed:" << m_program->log();
            cleanup();
            return false;
        }

        m_program->bind();
        m_texLoc = m_program->uniformLocation("tex");
        m_alphaLoc = m_program->uniformLocation("alpha");
        m_cameraMatLoc = m_program->uniformLocation("cameraMat");
        m_program->release();

        // 创建 VAO/VBO/EBO
        m_gl->glGenVertexArrays(1, &m_vao);
        m_gl->glGenBuffers(1, &m_vbo);
        m_gl->glGenBuffers(1, &m_ebo);

        m_gl->glBindVertexArray(m_vao);

        // 顶点属性：位置 (location=0)
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        // 顶点属性：纹理坐标 (location=1)
        m_gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
            (void*)(2 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(1);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        m_gl->glBindVertexArray(0);

        return true;
    }

    void TextureRenderer::updateData(const std::vector<TextureData>& vTexDatas)
    {
        if (!m_gl || !m_vao) return;

        // 在更新新数据前，先释放旧的纹理资源
        for (GLuint texId : m_textureIds)
        {
            if (texId > 0)
            {
                m_gl->glDeleteTextures(1, &texId);
            }
        }

        m_batches.clear();
        m_textureIds.clear();
        size_t totalVertices = 0, totalIndices = 0;

        // 预计算批次信息
        for (const auto& texData : vTexDatas)
        {
            Batch batch;
            batch.indexOffset = totalIndices;
            batch.indexCount = texData.indices.size();
            batch.textureId = texData.textureId;
            batch.brush = texData.brush;
            m_batches.push_back(batch);

            // 收集纹理ID以便后续清理
            m_textureIds.push_back(texData.textureId);

            totalVertices += texData.vertices.size() / 4; // 每个顶点4个float
            totalIndices += texData.indices.size();
        }

        // 合并顶点数据 (x, y, u, v)
        std::vector<float> allVertices;
        allVertices.reserve(totalVertices * 4);
        for (const auto& texData : vTexDatas)
        {
            allVertices.insert(allVertices.end(),
                texData.vertices.begin(), texData.vertices.end());
        }

        // 合并索引数据（重新计算偏移）
        std::vector<unsigned int> allIndices;
        allIndices.reserve(totalIndices);

        unsigned int vertexOffset = 0;
        for (const auto& texData : vTexDatas)
        {
            for (unsigned int idx : texData.indices)
            {
                allIndices.push_back(idx + vertexOffset);
            }
            vertexOffset += texData.vertices.size() / 4;
        }

        // 上传GPU
        m_gl->glBindVertexArray(m_vao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float),
            allVertices.data(), GL_STATIC_DRAW);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(unsigned int),
            allIndices.data(), GL_STATIC_DRAW);

        m_gl->glBindVertexArray(0);
    }

    void TextureRenderer::render(const float* cameraMat)
    {
        if (!m_gl || !m_program || m_batches.empty()) return;

        m_program->bind();
        m_gl->glBindVertexArray(m_vao);

        // 启用混合
        m_gl->glEnable(GL_BLEND);
        m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 设置相机矩阵
        //if (m_cameraMatLoc >= 0)
        //{
        //    QMatrix4x4 mat = cameraMat ? QMatrix4x4(cameraMat).transposed()
        //        : QMatrix4x4();
        //    m_program->setUniformValue(m_cameraMatLoc, mat);
        //}

        float identity[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
        if (m_cameraMatLoc >= 0)
        {
            QMatrix4x4 mat(identity);
            m_program->setUniformValue(m_cameraMatLoc, mat);
        }

        // 激活纹理单元 0
        m_gl->glActiveTexture(GL_TEXTURE0);
        m_program->setUniformValue(m_texLoc, 0); // 绑定到 uniform sampler2D

        // 遍历批次（每个纹理1次draw call）
        for (const auto& batch : m_batches)
        {
            // 绑定纹理
            m_gl->glBindTexture(GL_TEXTURE_2D, batch.textureId);

            // 设置 alpha
            if (m_alphaLoc >= 0)
            {
                m_program->setUniformValue(m_alphaLoc, batch.brush.a());
            }

            // 绘制
            m_gl->glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT,
                (void*)(batch.indexOffset * sizeof(unsigned int)));
        }

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void TextureRenderer::cleanup()
    {
        if (!m_gl)
            return;

        // 释放纹理资源
        for (GLuint texId : m_textureIds)
        {
            if (texId > 0)
            {
                m_gl->glDeleteTextures(1, &texId);
            }
        }
        m_textureIds.clear();

        if (m_vao)
        {
            m_gl->glDeleteVertexArrays(1, &m_vao);
            m_gl->glDeleteBuffers(1, &m_vbo);
            m_gl->glDeleteBuffers(1, &m_ebo);
            m_vao = m_vbo = m_ebo = 0;
        }

        delete m_program;
        m_program = nullptr;
        m_gl = nullptr;
    }
}