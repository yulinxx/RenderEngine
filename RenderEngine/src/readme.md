main.cpp

#include <QApplication>
#include "viewwrapper.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ViewWrapper window;
    window.resize(800, 600);
    window.show();
    return app.exec();
}

viewwrapper.h

#ifndef VIEWWRAPPER_H
#define VIEWWRAPPER_H

#include <QWidget>
#include "glZoomView.h"

class ViewWrapper : public QWidget
{
    Q_OBJECT
public:
    explicit ViewWrapper(QWidget *parent = nullptr);
    ~ViewWrapper();

private:
    glZoomView* m_glView{ nullptr };
};

#endif // VIEWWRAPPER_H


viewwrapper.cpp
#include "viewwrapper.h"
#include <QVBoxLayout>

ViewWrapper::ViewWrapper(QWidget *parent) : QWidget(parent)
{
    m_glView = new glZoomView(this);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_glView);
    setLayout(layout);
}

ViewWrapper::~ViewWrapper()
{
}

glZoomView.h

#ifndef MARCHVIEW_H
#define MARCHVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLShaderProgram>
#include <QVector2D>

#include <vector>
#include <QPainter> // 添加 QPainter 支持

class glZoomView : public QOpenGLWidget, protected QOpenGLFunctions_4_0_Core
{
    Q_OBJECT
public:
    explicit glZoomView(QWidget *parent = nullptr);
    ~glZoomView();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void paintEvent(QPaintEvent *event) override; // 添加 paintEvent 用于绘制文本

    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void updateLineBuffer();
    void updateCrossBuffer();
    void updateRuler();

private:
    QOpenGLShaderProgram *m_lineProgram = nullptr;
    GLuint m_lineVao;
    GLuint m_lineVbo;

    float m_scale = 1.0f;
    QVector2D m_translation;
    QPoint m_lastPos;

    struct Point {
        float x, y, z;
        float r, g, b;
    };

    std::vector<Point> m_linePoints;

    std::vector<Point> m_crossPoints;
    GLuint m_crossVao, m_crossVbo;
    QOpenGLShaderProgram* m_crossProgram;

    struct RulerLine {
        Point start;
        Point end;
        float worldValue; // 添加世界坐标值用于文本显示
    };
    std::vector<RulerLine> m_rulerLines;
    GLuint m_rulerVao, m_rulerVbo;
    QOpenGLShaderProgram* m_rulerProgram;
};

#endif // MARCHVIEW_H

glZoomView.cpp



#include "glZoomView.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QOpenGLShaderProgram>
#include <QDebug>
#include <cmath>
#include <random>

glZoomView::glZoomView(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMouseTracking(true);

    QSurfaceFormat format;
    format.setVersion(4, 0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disColor(0.0f, 1.0f);
    
    m_crossPoints = {
        {-0.9f, 0.0f, 0.0f, float(disColor(gen)), float(disColor(gen)), float(disColor(gen))},
        {0.9f, 0.0f, 0.0f, float(disColor(gen)), float(disColor(gen)), float(disColor(gen))},
        {0.0f, -0.9f, 0.0f, float(disColor(gen)), float(disColor(gen)), float(disColor(gen))},
        {0.0f, 0.9f, 0.0f, float(disColor(gen)), float(disColor(gen)), float(disColor(gen))}
    };

    // 设置焦点策略，确保能够接收键盘事件
    setFocusPolicy(Qt::StrongFocus);
}

glZoomView::~glZoomView()
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

void glZoomView::initializeGL()
{
    if (!initializeOpenGLFunctions())
    {
        qFatal("Could not initialize OpenGL 4.0 functions");
    }

    // 修改顶点着色器以处理新的数据类型
    const char *vertexShaderSource = R"(
        #version 400
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        uniform mat4 projection;
        uniform vec2 translation;
        uniform float scale;
        out vec3 fragColor;
        void main()
        {
            vec3 scaledPos = vec3(position.xy * scale + translation, position.z);
            gl_Position = projection * vec4(scaledPos, 1.0);
            fragColor = color;
        }
    )";
    
    // 修改片段着色器以接收颜色信息
    const char *fragmentShaderSource = R"(
        #version 400
        in vec3 fragColor;
        out vec4 outColor;
        void main()
        {
            outColor = vec4(fragColor, 1.0);
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
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 修改 cross 的顶点着色器以处理新的数据类型
    const char *crossVS = R"(
        #version 400
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        out vec3 fragColor;
        void main()
        {
            gl_Position = vec4(position, 1.0);
            fragColor = color;
        }
    )";
    
    // 修改 cross 的片段着色器以接收颜色信息
    const char *crossFS = R"(
        #version 400
        in vec3 fragColor;
        out vec4 outColor;
        void main()
        {
            outColor = vec4(fragColor, 1.0);
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
    // 修改顶点属性指针以处理新的数据类型
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    const char *rulerVS = R"(
        #version 400
        layout(location = 0) in vec2 position;
        void main()
        {
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";
    const char *rulerFS = R"(
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

void glZoomView::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void glZoomView::paintGL()
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
        //glDrawArrays(GL_LINE_STRIP, 0, m_linePoints.size());
        glDrawArrays(GL_LINES, 0, m_linePoints.size());
        glBindVertexArray(0);
        m_lineProgram->release();
    }

    if (m_crossPoints.size() >= 2 && m_crossProgram->bind())
    {
        glBindVertexArray(m_crossVao);
        glDrawArrays(GL_LINES, 0, m_crossPoints.size());
        glBindVertexArray(0);
        m_crossProgram->release();
    }

    if (m_rulerLines.size() >= 1 && m_rulerProgram->bind())
    {
        glBindVertexArray(m_rulerVao);
        glDrawArrays(GL_LINES, 0, m_rulerLines.size() * 2);
        glBindVertexArray(0);
        m_rulerProgram->release();
    }
}

void glZoomView::paintEvent(QPaintEvent *event)
{
    // 先调用基类的 paintEvent，执行 OpenGL 绘制
    QOpenGLWidget::paintEvent(event);

    // 使用 QPainter 在 OpenGL 渲染后绘制文本
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QFont font("Arial", 8);
    painter.setFont(font);
    painter.setPen(Qt::black);

    // 绘制标尺文本
    for (const auto &line : m_rulerLines)
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

void glZoomView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // 获取视口的宽高
        float aspect = float(width()) / height();
        float orthoSize = 1000.0f;

        // 将鼠标的屏幕坐标转换为归一化设备坐标（NDC）
        float ndcX = (float(event->x()) / width()) * 2.0f - 1.0f;
        float ndcY = -((float(event->y()) / height()) * 2.0f - 1.0f);

        //// 将 NDC 坐标转换为世界坐标，考虑缩放和偏移
        //float worldX = (ndcX * orthoSize * aspect - m_translation.x()) / m_scale;
        //float worldY = (ndcY * orthoSize - m_translation.y()) / m_scale;

        // NDC 转换为投影坐标（考虑正交投影范围）
        float scaledX = ndcX * orthoSize * aspect;
        float scaledY = ndcY * orthoSize;

        // 应用缩放和平移的逆操作
        float worldX = (scaledX - m_translation.x()) / m_scale;
        float worldY = (scaledY - m_translation.y()) / m_scale;

        // 随机生成颜色
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> disColor(0.0f, 1.0f);
        float r = disColor(gen);
        float g = disColor(gen);
        float b = disColor(gen);

        // 将新的点添加到线的点列表中
        m_linePoints.push_back({worldX, worldY, 0.0f, r, g, b});

        // 更新线的缓冲区
        updateLineBuffer();

        // 请求重绘
        update();
    }
    else if (event->button() == Qt::RightButton)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> disColor(0.0f, 1.0f);

        // 获取当前的缩放和偏移
        float aspect = float(width()) / height();
        float orthoSize = 1000.0f;

        // 计算投影边界的原始范围（不考虑缩放和平移）
        float scaledLeft = -orthoSize * aspect;
        float scaledRight = orthoSize * aspect;
        float scaledBottom = -orthoSize;
        float scaledTop = orthoSize;

        // 根据当前缩放和平移计算有效世界坐标范围
        float left = (scaledLeft - m_translation.x()) / m_scale;
        float right = (scaledRight - m_translation.x()) / m_scale;
        float bottom = (scaledBottom - m_translation.y()) / m_scale;
        float top = (scaledTop - m_translation.y()) / m_scale;

        // 生成随机点时使用修正后的范围
        std::uniform_real_distribution<float> disX(left, right);
        std::uniform_real_distribution<float> disY(bottom, top);

        // 生成随机点并添加到 m_linePoints 中
        for (int i = 0; i < 100000; ++i)
        {
            float randomX = disX(gen);
            float randomY = disY(gen);
            float r = disColor(gen);
            float g = disColor(gen);
            float b = disColor(gen);
            m_linePoints.push_back({randomX, randomY, 0.0f, r, g, b});
        }

        // 更新线的缓冲区并请求重绘
        updateLineBuffer();
        update();

        //std::random_device rd;
        //std::mt19937 gen(rd());
        //std::uniform_real_distribution<> disColor(0.0f, 1.0f);

        //float aspect = float(width()) / height();
        //float orthoSize = 1000.0f;
        //float viewWidth = orthoSize * aspect * 2.0f / m_scale;
        //float viewHeight = orthoSize * 2.0f / m_scale;

        //float left = -viewWidth / 2.0f + m_translation.x();
        //float right = viewWidth / 2.0f + m_translation.x();
        //float bottom = -viewHeight / 2.0f + m_translation.y();
        //float top = viewHeight / 2.0f + m_translation.y();

        //std::uniform_real_distribution<> disX(left, right);
        //std::uniform_real_distribution<> disY(bottom, top);

        //for (int i = 0; i < 100000; ++i)
        //{
        //    float randomX = disX(gen);
        //    float randomY = disY(gen);
        //    float r = float(disColor(gen));
        //    float g = float(disColor(gen));
        //    float b = float(disColor(gen));
        //    m_linePoints.push_back({randomX, randomY, 0.0f, r, g, b});
        //}

        //updateLineBuffer();
        //update();
    }
    else if (event->button() == Qt::MiddleButton)
    {
        m_lastPos = event->pos();
    }
}

void glZoomView::wheelEvent(QWheelEvent *event)
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

void glZoomView::mouseMoveEvent(QMouseEvent *event)
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


void glZoomView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        // 清空 m_linePoints 并回收内存
        m_linePoints.clear();
        m_linePoints.shrink_to_fit();

        updateLineBuffer();
        update();
    }
    else
    {
        // 调用基类的 keyPressEvent 处理其他按键事件
        QOpenGLWidget::keyPressEvent(event);
    }
}

void glZoomView::updateLineBuffer()
{
    glBindVertexArray(m_lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
    if (m_linePoints.empty()) 
    {
        // 如果 m_linePoints 为空，释放 VBO 的内存
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

        //glDeleteBuffers(1, &m_lineVbo);
        //glGenBuffers(1, &m_lineVbo);
        //glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
    }
     else 
     {
        glBufferData(GL_ARRAY_BUFFER, m_linePoints.size() * sizeof(Point),
                     m_linePoints.data(), GL_STATIC_DRAW);
    }
    
    // 修改顶点属性指针以处理新的数据类型
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glZoomView::updateCrossBuffer()
{
    glBindVertexArray(m_crossVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_crossVbo);
    // 显式转换为 GLsizei 类型
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(m_crossPoints.size() * sizeof(Point)),
                 m_crossPoints.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glZoomView::updateRuler()
{
    return;

    m_rulerLines.clear();
    m_rulerLines.shrink_to_fit();

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
        return {ndcX, ndcY};
    };

    float rulerHeight = 0.1f;
    for (float x = floor(left / step) * step; x <= right; x += step)
    {
        Point start = worldToNDC(x, bottom);
        start.y = -1.0f;
        Point end = start;
        end.y += rulerHeight;
        if (start.x >= -1.0f && start.x <= 1.0f)
            m_rulerLines.push_back({start, end, x}); // 存储世界坐标值
    }

    float rulerWidth = 0.1f;
    for (float y = floor(bottom / step) * step; y <= top; y += step)
    {
        Point start = worldToNDC(left, y);
        start.x = -1.0f;
        Point end = start;
        end.x += rulerWidth;
        if (start.y >= -1.0f && start.y <= 1.0f)
            m_rulerLines.push_back({start, end, y}); // 存储世界坐标值
    }

    std::vector<Point> rulerPoints;
    for (const auto &line : m_rulerLines)
    {
        rulerPoints.push_back(line.start);
        rulerPoints.push_back(line.end);
    }

    glBindVertexArray(m_rulerVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_rulerVbo);
    // 显式转换为 GLsizei 类型
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(rulerPoints.size() * sizeof(Point)),
                 rulerPoints.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


一个 viewWrapper 包了一个 glZoomView

现在我想在 viewWrapper 中进行鼠标操作， 比如 左/右键点击， 中键滚轮缩放 ， 按住中键移动
 glZoomView， 而不是在 glZoomView 中操作了。

并且我能在 viewWrapper  获取 到 glZoomView 的各种点坐标 缩放 视图坐标等，  这个能实现吗， 
帮我写出完整 代码以及详细注释