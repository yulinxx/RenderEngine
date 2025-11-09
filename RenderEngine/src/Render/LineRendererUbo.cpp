#include "Render/LineRendererUbo.h"

#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QDebug>
#include <array>

static const char* vertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in float aLineID;

uniform mat4 uCameraMatrix;

layout(std140) uniform LineDataUBO {
    vec4 colors[2048];
    float depths[2048];
};

flat out vec4 vColor;
out float vDepth;

void main()
{
    int id = int(aLineID);
    vColor = colors[id];
    vDepth = depths[id];
    gl_Position = uCameraMatrix * vec4(aPos.xy, vDepth, 1.0);
}
)";

static const char* fragmentShader = R"(
#version 330 core
flat in vec4 vColor;
out vec4 fragColor;

void main()
{
    fragColor = vColor;
}
)";

namespace GLRhi
{
    bool LineRendererUbo::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl) return false;

        m_program = new QOpenGLShaderProgram;
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader) ||
            !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader) ||
            !m_program->link())
        {
            qCritical() << "Shader error:" << m_program->log();
            delete m_program; m_program = nullptr;
            return false;
        }

        m_cameraMatLoc = m_program->uniformLocation("uCameraMatrix");
        m_gl->glGenBuffers(1, &m_nUbo);

        qDebug() << "LineRendererUbo initialized";
        return true;
    }

    void LineRendererUbo::cleanup()
    {
        if (!m_gl)
            return;

        deleteVbo(m_nVbo);
        deleteEbo(m_nEbo);
        deleteVao(m_nVao);
        deleteProgram(m_program);

        m_totalIndexCount = 0;

        m_batches.clear();
    }

    void LineRendererUbo::render(const float* cameraMat)
    {
        if (!m_nVao || !m_totalIndexCount || !m_program)
        {
            qWarning() << "Render: Not ready!";
            return;
        }

        m_program->bind();
        m_gl->glBindVertexArray(m_nVao);

        // 绑定UBO
        GLuint nBlockIndex = m_gl->glGetUniformBlockIndex(m_program->programId(), "LineDataUBO");
        if (nBlockIndex != GL_INVALID_INDEX)
        {
            m_gl->glUniformBlockBinding(m_program->programId(), nBlockIndex, 0);
            m_gl->glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_nUbo);
        }

        // 使用单位矩阵
        float identity[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
        if (m_cameraMatLoc >= 0)
        {
            QMatrix4x4 mat(identity);
            m_program->setUniformValue(m_cameraMatLoc, mat);
        }

        // 强制可见状态
        m_gl->glDisable(GL_DEPTH_TEST);
        m_gl->glEnable(GL_LINE_SMOOTH);

        // 绘制
        m_gl->glEnable(GL_PRIMITIVE_RESTART);
        m_gl->glPrimitiveRestartIndex(0xFFFFFFFF);
        m_gl->glDrawElements(GL_LINE_STRIP, m_totalIndexCount, GL_UNSIGNED_INT, nullptr);
        m_gl->glDisable(GL_PRIMITIVE_RESTART);

        m_gl->glBindVertexArray(0);
        m_program->release();
    }

    void LineRendererUbo::updateData(const std::vector<PolylineData>& polylines)
    {
        if (!m_gl || polylines.empty())
        {
            cleanup();
            return;
        }

        if (polylines.size() > 2048)
        {
            qCritical() << "线段组数量超出UBO容量:" << polylines.size();
            return;
        }

        deleteVbo(m_nVbo);
        deleteEbo(m_nEbo);
        deleteVao(m_nVao);

        // 预计算总大小
        size_t totalVertices = 0;
        size_t totalIndices = 0;
        for (const auto& p : polylines)
        {
            for (int count : p.lineVertexCounts)
            {
                if (count >= 2)
                {
                    totalVertices += count;
                    totalIndices += count + 1;
                }
            }
        }

        if (totalVertices == 0)
        {
            cleanup();
            return;
        }

        m_totalIndexCount = 0;
        m_batches.clear();

        // 顶点数据：位置 + 线段组ID
        std::vector<float> vertexData;
        std::vector<GLuint> indexData;
        vertexData.reserve(totalVertices * 4); // 3 pos + 1 lineID
        indexData.reserve(totalIndices);

        // UBO数据（每段线一组颜色）
        std::vector<float> uboColors(polylines.size() * 4);
        std::vector<float> uboDepths(polylines.size());

        GLuint vertexOffset = 0;
        GLuint indexOffset = 0;

        for (size_t groupID = 0; groupID < polylines.size(); ++groupID)
        {
            const auto& group = polylines[groupID];
            if (group.lineVertexCounts.empty()) continue;

            // 存储UBO（整个线段组共享）
            uboColors[groupID * 4 + 0] = group.brush.r();
            uboColors[groupID * 4 + 1] = group.brush.g();
            uboColors[groupID * 4 + 2] = group.brush.b();
            uboColors[groupID * 4 + 3] = group.brush.a();
            uboDepths[groupID] = group.brush.d();

            // 处理该组内的每一条线段
            int vertexIndexInGroup = 0;
            for (int lineVertexCount : group.lineVertexCounts)
            {
                if (lineVertexCount < 2)
                {
                    vertexIndexInGroup += lineVertexCount;
                    continue;
                }

                // 索引：当前线段的顶点
                for (int i = 0; i < lineVertexCount; ++i)
                {
                    indexData.push_back(vertexOffset + static_cast<GLuint>(i));
                }
                indexData.push_back(0xFFFFFFFF); // 图元重启，分隔线段

                // 记录批次信息（调试用）
                m_batches.push_back({
                    static_cast<GLuint>(lineVertexCount + 1), // 包含重启索引
                    indexOffset,
                    static_cast<GLuint>(groupID)
                    });

                vertexOffset += static_cast<GLuint>(lineVertexCount);
                indexOffset += static_cast<GLuint>(lineVertexCount) + 1;
            }

            // 顶点：位置 + 所属线段组ID
            for (size_t i = 0; i < group.vertices.size() / 3; ++i)
            {
                vertexData.push_back(group.vertices[i * 3 + 0]);
                vertexData.push_back(group.vertices[i * 3 + 1]);
                vertexData.push_back(group.vertices[i * 3 + 2]);
                // 所属线段组ID（该组内所有顶点相同）
                vertexData.push_back(static_cast<float>(groupID));
            }
        }

        m_totalIndexCount = static_cast<GLsizei>(indexData.size());

        // 上传UBO
        m_gl->glBindBuffer(GL_UNIFORM_BUFFER, m_nUbo);
        m_gl->glBufferData(GL_UNIFORM_BUFFER, 2048 * 4 * sizeof(float) + 2048 * sizeof(float), nullptr, GL_STATIC_DRAW);
        m_gl->glBufferSubData(GL_UNIFORM_BUFFER, 0, uboColors.size() * sizeof(float), uboColors.data());
        m_gl->glBufferSubData(GL_UNIFORM_BUFFER, 2048 * 4 * sizeof(float), uboDepths.size() * sizeof(float), uboDepths.data());
        m_gl->glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // 创建VAO/VBO/EBO
        m_gl->glGenVertexArrays(1, &m_nVao);
        m_gl->glBindVertexArray(m_nVao);

        m_gl->glGenBuffers(1, &m_nVbo);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, m_nVbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

        // 位置属性
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        // 线段组ID属性
        m_gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
        m_gl->glEnableVertexAttribArray(1);

        // EBO
        m_gl->glGenBuffers(1, &m_nEbo);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nEbo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), indexData.data(), GL_STATIC_DRAW);

        m_gl->glBindVertexArray(0);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        qDebug() << "LineRenderer: Uploaded" << polylines.size() << "groups,"
            << (vertexData.size() / 4) << "vertices," << indexData.size() << "indices";
    }
}