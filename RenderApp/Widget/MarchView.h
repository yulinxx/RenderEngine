#ifndef MARCHVIEW_H
#define MARCHVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLShaderProgram>
#include <QVector2D>
#include <vector>
#include <QPainter>
namespace GLRhi
{
    class MarchView : public QOpenGLWidget, protected QOpenGLFunctions_4_0_Core
    {
        Q_OBJECT
    public:
        explicit MarchView(QWidget* parent = nullptr);
        ~MarchView();

    protected:
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;
        void paintEvent(QPaintEvent* event) override; // 添加 paintEvent 用于绘制文本

        void mousePressEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;

    private:
        void updateLineBuffer();
        void updateCrossBuffer();
        void updateRuler();

    private:
        QOpenGLShaderProgram* m_lineProgram = nullptr;
        GLuint m_lineVao;
        GLuint m_lineVbo;

        float m_scale = 1.0f;
        QVector2D m_translation;
        QPoint m_lastPos;

        struct Point
        {
            float x, y;
        };
        std::vector<Point> m_linePoints;

        std::vector<Point> m_crossPoints;
        GLuint m_crossVao, m_crossVbo;
        QOpenGLShaderProgram* m_crossProgram;

        struct RulerLine
        {
            Point start;
            Point end;
            float worldValue; // 添加世界坐标值用于文本显示
        };
        std::vector<RulerLine> m_rulerLines;
        GLuint m_rulerVao, m_rulerVbo;
        QOpenGLShaderProgram* m_rulerProgram;
    };
}
#endif // MARCHVIEW_H