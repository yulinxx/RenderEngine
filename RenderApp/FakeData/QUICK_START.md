# 三角形渲染系统 - 快速开始

## 5分钟快速上手

### 第一步：包含必要的头文件

```cpp
#include "FakeData/FakeTriangle.h"
#include "FakeData/FakeTriangleHelper.h"
#include "Render/TriangleRenderer.h"
```

### 第二步：初始化渲染器（在 initializeGL 中）

```cpp
void YourWidget::initializeGL()
{
    QOpenGLFunctions_3_3_Core* gl = 
        QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    
    m_triangleRenderer = new GLRhi::TriangleRenderer();
    m_triangleRenderer->initialize(gl);
}
```

### 第三步：生成数据（在 initializeGL 或任何需要的时候）

```cpp
void YourWidget::generateTriangles()
{
    // 创建生成器
    GLRhi::FakeTriangle fakeTriangle;
    
    // 生成20个多边形，每个3-12个顶点，50%为凹多边形
    fakeTriangle.generatePolygons(20, 3, 12, 0.5f);
    
    // 设置颜色（橙色）
    GLRhi::Brush brush(0.8f, 0.4f, 0.1f, 1.0f, 0.0f);
    
    // 转换数据
    GLRhi::TriangleData data = GLRhi::convertToTriangleData(fakeTriangle, 1, brush);
    
    // 上传到渲染器
    m_triangleRenderer->updateData({data});
}
```

### 第四步：渲染（在 paintGL 中）

```cpp
void YourWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 设置相机矩阵（这里使用单位矩阵作为示例）
    float cameraMat[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    m_triangleRenderer->render(cameraMat);
}
```

### 第五步：清理（在析构或 cleanup 中）

```cpp
void YourWidget::cleanup()
{
    if (m_triangleRenderer)
    {
        m_triangleRenderer->cleanup();
        delete m_triangleRenderer;
        m_triangleRenderer = nullptr;
    }
}
```

---

## 完整的类定义示例

```cpp
// YourWidget.h
class YourWidget : public QOpenGLWidget
{
    Q_OBJECT
    
public:
    explicit YourWidget(QWidget* parent = nullptr);
    ~YourWidget() override;

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    GLRhi::TriangleRenderer* m_triangleRenderer = nullptr;
};

// YourWidget.cpp
#include "YourWidget.h"
#include "FakeData/FakeTriangle.h"
#include "FakeData/FakeTriangleHelper.h"
#include "Render/TriangleRenderer.h"

YourWidget::YourWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

YourWidget::~YourWidget()
{
    makeCurrent();
    if (m_triangleRenderer)
    {
        m_triangleRenderer->cleanup();
        delete m_triangleRenderer;
    }
    doneCurrent();
}

void YourWidget::initializeGL()
{
    QOpenGLFunctions_3_3_Core* gl = 
        QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    
    // 设置背景色
    gl->glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // 初始化渲染器
    m_triangleRenderer = new GLRhi::TriangleRenderer();
    m_triangleRenderer->initialize(gl);
    
    // 生成测试数据
    GLRhi::FakeTriangle fakeTriangle;
    fakeTriangle.generatePolygons(20, 3, 12, 0.5f);
    
    GLRhi::Brush brush(0.8f, 0.4f, 0.1f, 1.0f, 0.0f);
    GLRhi::TriangleData data = GLRhi::convertToTriangleData(fakeTriangle, 1, brush);
    
    m_triangleRenderer->updateData({data});
}

void YourWidget::paintGL()
{
    QOpenGLFunctions_3_3_Core* gl = 
        QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    float cameraMat[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    
    m_triangleRenderer->render(cameraMat);
}

void YourWidget::resizeGL(int w, int h)
{
    QOpenGLFunctions_3_3_Core* gl = 
        QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    gl->glViewport(0, 0, w, h);
}
```

---

## 常见问题

### Q: 为什么看不到任何东西？

A: 检查以下几点：
1. 确保相机矩阵正确
2. 检查生成的多边形是否在视口范围内
3. 确认背景色和多边形颜色不同
4. 检查深度测试设置

### Q: 如何更改颜色？

A: 修改 Brush 参数：
```cpp
GLRhi::Brush redBrush(1.0f, 0.0f, 0.0f, 1.0f, 0.0f);    // 红色
GLRhi::Brush greenBrush(0.0f, 1.0f, 0.0f, 1.0f, 0.0f);  // 绿色
GLRhi::Brush blueBrush(0.0f, 0.0f, 1.0f, 1.0f, 0.0f);   // 蓝色
```

### Q: 如何生成更多/更少的多边形？

A: 修改 generatePolygons 的第一个参数：
```cpp
fakeTriangle.generatePolygons(50, 3, 12, 0.5f);  // 生成50个多边形
fakeTriangle.generatePolygons(5, 3, 12, 0.5f);   // 生成5个多边形
```

### Q: 如何只生成凸多边形？

A: 将 concaveRatio 设为 0.0：
```cpp
fakeTriangle.generatePolygons(20, 3, 12, 0.0f);  // 全部凸多边形
```

### Q: 如何只生成凹多边形？

A: 将 concaveRatio 设为 1.0：
```cpp
fakeTriangle.generatePolygons(20, 3, 12, 1.0f);  // 全部凹多边形
```

### Q: 如何生成多组不同颜色的多边形？

A: 创建多个 TriangleData 并一起上传：
```cpp
std::vector<GLRhi::TriangleData> triangleDatas;

// 红色组
GLRhi::FakeTriangle red;
red.generatePolygons(10, 3, 8, 0.3f);
triangleDatas.push_back(GLRhi::convertToTriangleData(
    red, 1, GLRhi::Brush(1.0f, 0.0f, 0.0f, 1.0f, 0.0f)
));

// 绿色组
GLRhi::FakeTriangle green;
green.generatePolygons(10, 3, 8, 0.5f);
triangleDatas.push_back(GLRhi::convertToTriangleData(
    green, 2, GLRhi::Brush(0.0f, 1.0f, 0.0f, 1.0f, 0.1f)
));

// 蓝色组
GLRhi::FakeTriangle blue;
blue.generatePolygons(10, 3, 8, 0.7f);
triangleDatas.push_back(GLRhi::convertToTriangleData(
    blue, 3, GLRhi::Brush(0.0f, 0.0f, 1.0f, 1.0f, 0.2f)
));

m_triangleRenderer->updateData(triangleDatas);
```

---

## 下一步

- 阅读 [TRIANGLE_RENDER_README.md](TRIANGLE_RENDER_README.md) 了解详细的技术细节
- 查看 [TriangleRenderExample.cpp](TriangleRenderExample.cpp) 获取更多示例
- 参考 [LineRenderer](../../RenderEngine/include/Render/LineRenderer.h) 了解渲染器设计模式

---

## 需要帮助？

如果遇到问题，请检查：
1. OpenGL 上下文是否正确初始化
2. 是否包含了所有必要的头文件
3. shader 是否正确编译（查看控制台输出）
4. 数据是否为空（查看 qDebug 输出）

