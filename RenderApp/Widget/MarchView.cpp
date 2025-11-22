#include "marchview.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QDebug>
#include <cmath>

namespace GLRhi
{
    MarchView::MarchView(QWidget* parent)
        : QOpenGLWidget(parent)
    {
        setMouseTracking(true);

        QSurfaceFormat format;
        format.setVersion(4, 0);
        format.setProfile(QSurfaceFormat::CoreProfile);
        setFormat(format);

        m_crossPoints = {
            {-0.9f, 0.0f}, {0.9f, 0.0f},
            {0.0f, -0.9f}, {0.0f, 0.9f}
        };
    }

    MarchView::~MarchView()
    {
        makeCurrent();
        glDeleteVertexArrays(1, &m_lineVao);
        glDeleteBuffers(1, &m_lineVbo);
        delete m_lineProgram;

        glDeleteVertexArrays(1, &m_crossVao);
        glDeleteBuffers(1, &m_crossVbo);
        delete m_crossProgram;

        glDeleteVertexArrays(1, &m_rulerVao);
        glDeleteBuffers(1, &m_rulerVbo);
        delete m_rulerProgram;

        doneCurrent();
    }

    void MarchView::initializeGL()
    {
        if (!initializeOpenGLFunctions())
        {
            qFatal("Could not initialize OpenGL 4.0 functions");
        }

        const char* vertexShaderSource = R"(
        #version 400
        layout(location = 0) in vec2 position;
        uniform mat4 projection;
        uniform vec2 translation;
        uniform float scale;
        void main()
        {
            vec2 scaledPos = position * scale + translation;
            gl_Position = projection * vec4(scaledPos, 0.0, 1.0);
        }
    )";
        const char* fragmentShaderSource = R"(
        #version 400
        out vec4 fragColor;
        void main()
        {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";
        m_lineProgram = new QOpenGLShaderProgram;
        m_lineProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
        m_lineProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
        m_lineProgram->link();
        glGenVertexArrays(1, &m_lineVao);
        glGenBuffers(1, &m_lineVbo);
        glBindVertexArray(m_lineVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        const char* crossVS = R"(
        #version 400
        layout(location = 0) in vec2 position;
        void main()
        {
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";
        const char* crossFS = R"(
        #version 400
        out vec4 fragColor;
        void main()
        {
            fragColor = vec4(0.0, 0.0, 1.0, 1.0);
        }
    )";
        m_crossProgram = new QOpenGLShaderProgram;
        m_crossProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, crossVS);
        m_crossProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, crossFS);
        m_crossProgram->link();
        glGenVertexArrays(1, &m_crossVao);
        glGenBuffers(1, &m_crossVbo);
        glBindVertexArray(m_crossVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_crossVbo);
        glBufferData(GL_ARRAY_BUFFER, m_crossPoints.size() * sizeof(Point),
            m_crossPoints.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        const char* rulerVS = R"(
        #version 400
        layout(location = 0) in vec2 position;
        void main()
        {
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";
        const char* rulerFS = R"(
        #version 400
        out vec4 fragColor;
        void main()
        {
            fragColor = vec4(0.0, 0.0, 1.0, 1.0);
        }
    )";
        m_rulerProgram = new QOpenGLShaderProgram;
        m_rulerProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, rulerVS);
        m_rulerProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, rulerFS);
        m_rulerProgram->link();
        glGenVertexArrays(1, &m_rulerVao);
        glGenBuffers(1, &m_rulerVbo);
        glBindVertexArray(m_rulerVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_rulerVbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        updateRuler();
    }

    void MarchView::resizeGL(int w, int h)
    {
        glViewport(0, 0, w, h);
    }

    void MarchView::paintGL()
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float aspect = float(width()) / height();
        float orthoSize = 1000.0f;
        QMatrix4x4 projection;
        projection.ortho(-orthoSize * aspect, orthoSize * aspect,
            -orthoSize, orthoSize,
            -1.0f, 1.0f);

        if (m_linePoints.size() >= 2 && m_lineProgram->bind())
        {
            m_lineProgram->setUniformValue("projection", projection);
            m_lineProgram->setUniformValue("translation", m_translation);
            m_lineProgram->setUniformValue("scale", m_scale);

            glBindVertexArray(m_lineVao);
            glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(m_linePoints.size()));
            glBindVertexArray(0);
            m_lineProgram->release();
        }

        if (m_crossPoints.size() >= 2 && m_crossProgram->bind())
        {
            glBindVertexArray(m_crossVao);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_crossPoints.size()));
            glBindVertexArray(0);
            m_crossProgram->release();
        }

        if (m_rulerLines.size() >= 1 && m_rulerProgram->bind())
        {
            glBindVertexArray(m_rulerVao);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_rulerLines.size() * 2));
            glBindVertexArray(0);
            m_rulerProgram->release();
        }
    }

    void MarchView::paintEvent(QPaintEvent* event)
    {
        QOpenGLWidget::paintEvent(event);

        // 使用 QPainter 在 OpenGL 渲染后绘制文本
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QFont font("Arial", 8);
        painter.setFont(font);
        painter.setPen(Qt::black);

        // 绘制标尺文本
        for (const auto& line : m_rulerLines)
        {
            // 将 NDC 坐标转换为屏幕像素坐标
            float screenX = (line.start.x + 1.0f) / 2.0f * width();
            float screenY = (1.0f - line.start.y) / 2.0f * height();

            // 根据标尺是水平还是垂直调整文本位置
            if (line.start.y == line.end.y) // 垂直标尺
            {
                painter.drawText(screenX + 5, screenY, QString::number(line.worldValue, 'f', 1));
            }
            else // 水平标尺
            {
                painter.drawText(screenX - 10, screenY - 5, QString::number(line.worldValue, 'f', 1));
            }
        }

        painter.end();
    }

    void MarchView::mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
        {
            float aspect = float(width()) / height();
            float orthoSize = 1000.0f;

            float ndcX = (float(event->x()) / width()) * 2.0f - 1.0f;
            float ndcY = -((float(event->y()) / height()) * 2.0f - 1.0f);

            float worldX = (ndcX * orthoSize * aspect - m_translation.x()) / m_scale;
            float worldY = (ndcY * orthoSize - m_translation.y()) / m_scale;

            m_linePoints.push_back({ worldX, worldY });
            updateLineBuffer();
            update();
        }
        else if (event->button() == Qt::MiddleButton)
        {
            m_lastPos = event->pos();
        }
    }

    void MarchView::wheelEvent(QWheelEvent* event)
    {
        float delta = event->angleDelta().y() > 0 ? 1.1f : 0.9f;
        QPointF pos = event->position();
        float aspect = float(width()) / height();
        float mouseX = (pos.x() / width() * 2.0f - 1.0f) * 1000.0f * aspect / m_scale - m_translation.x();
        float mouseY = -(pos.y() / height() * 2.0f - 1.0f) * 1000.0f / m_scale - m_translation.y();

        m_scale *= delta;
        m_translation.setX(m_translation.x() - mouseX * (delta - 1.0f) * m_scale);
        m_translation.setY(m_translation.y() - mouseY * (delta - 1.0f) * m_scale);

        updateRuler();
        update();
    }

    void MarchView::mouseMoveEvent(QMouseEvent* event)
    {
        if (event->buttons() & Qt::MiddleButton)
        {
            QPoint delta = event->pos() - m_lastPos;
            float aspect = float(width()) / height();
            float moveSpeed = 2.0f;
            float dx = delta.x() * moveSpeed * aspect / width();
            float dy = -delta.y() * moveSpeed / height();

            m_translation += QVector2D(dx, dy) * 1000.0f;
            m_lastPos = event->pos();

            updateRuler();
            update();
        }
    }

    void MarchView::updateLineBuffer()
    {
        glBindVertexArray(m_lineVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
        glBufferData(GL_ARRAY_BUFFER, m_linePoints.size() * sizeof(Point),
            m_linePoints.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void MarchView::updateCrossBuffer()
    {
        glBindVertexArray(m_crossVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_crossVbo);
        glBufferData(GL_ARRAY_BUFFER, m_crossPoints.size() * sizeof(Point),
            m_crossPoints.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void MarchView::updateRuler()
    {
        m_rulerLines.clear();

        float aspect = float(width()) / height();
        float orthoSize = 1000.0f;
        float viewWidth = orthoSize * aspect * 2.0f / m_scale;
        float viewHeight = orthoSize * 2.0f / m_scale;

        float left = -viewWidth / 2.0f + m_translation.x();
        float right = viewWidth / 2.0f + m_translation.x();
        float bottom = -viewHeight / 2.0f + m_translation.y();
        float top = viewHeight / 2.0f + m_translation.y();

        float step = pow(10.0f, floor(log10(viewWidth / 10.0f)));
        if (viewWidth / step > 20) step *= 2.0f;

        auto worldToNDC = [&](float worldX, float worldY) -> Point {
            float ndcX = (worldX - left) / (right - left) * 2.0f - 1.0f;
            float ndcY = (worldY - bottom) / (top - bottom) * 2.0f - 1.0f;
            return { ndcX, ndcY };
            };

        float rulerHeight = 0.1f;
        for (float x = floor(left / step) * step; x <= right; x += step)
        {
            Point start = worldToNDC(x, bottom);
            start.y = -1.0f;
            Point end = start;
            end.y += rulerHeight;
            if (start.x >= -1.0f && start.x <= 1.0f)
                m_rulerLines.push_back({ start, end, x }); // 存储世界坐标值
        }

        float rulerWidth = 0.1f;
        for (float y = floor(bottom / step) * step; y <= top; y += step)
        {
            Point start = worldToNDC(left, y);
            start.x = -1.0f;
            Point end = start;
            end.x += rulerWidth;
            if (start.y >= -1.0f && start.y <= 1.0f)
                m_rulerLines.push_back({ start, end, y }); // 存储世界坐标值
        }

        std::vector<Point> rulerPoints;
        for (const auto& line : m_rulerLines)
        {
            rulerPoints.push_back(line.start);
            rulerPoints.push_back(line.end);
        }

        glBindVertexArray(m_rulerVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_rulerVbo);
        glBufferData(GL_ARRAY_BUFFER, rulerPoints.size() * sizeof(Point),
            rulerPoints.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}