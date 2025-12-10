# 三角形生成与渲染系统

## 概述

这个系统提供了完整的多边形生成、三角剖分和渲染功能，包括以下核心组件：

1. **FakeTriangle** - 随机多边形生成器
2. **TriangleRenderer** - 三角形渲染器
3. **FakeTriangleHelper** - 数据转换辅助工具

---

## 核心类

### 1. FakeTriangle (数据生成器)

**文件位置：**
- `RenderApp/FakeData/FakeTriangle.h`
- `RenderApp/FakeData/FakeTriangle.cpp`

**功能：**
- 随机生成凸多边形和凹多边形
- 使用 [Earcut](https://github.com/mapbox/earcut.hpp) 库进行三角剖分
- 输出顶点数据和索引数据

**主要方法：**

```cpp
// 生成多边形
void generatePolygons(
    size_t polygonCount = 1,     // 生成的多边形数量
    size_t minVertices = 3,       // 每个多边形的最小顶点数
    size_t maxVertices = 20,      // 每个多边形的最大顶点数
    float concaveRatio = 0.5f     // 凹多边形比例 (0.0-1.0)
);

// 获取数据
const std::vector<float>& getVertices() const;        // 顶点数据 (x, y, z)
const std::vector<uint32_t>& getIndices() const;      // 索引数据
const std::vector<size_t>& getPolygonInfos() const;   // 多边形信息
```

**使用示例：**

```cpp
GLRhi::FakeTriangle fakeTriangle;

// 设置生成范围
fakeTriangle.setRange(-1.0f, 1.0f, -1.0f, 1.0f);

// 生成10个多边形，每个3-10个顶点，50%为凹多边形
fakeTriangle.generatePolygons(10, 3, 10, 0.5f);

// 获取结果
const auto& vertices = fakeTriangle.getVertices();  // x, y, z, x, y, z, ...
const auto& indices = fakeTriangle.getIndices();    // 三角形索引
```

---

### 2. TriangleRenderer (渲染器)

**文件位置：**
- `RenderEngine/include/Render/TriangleRenderer.h`
- `RenderEngine/src/Render/TriangleRenderer.cpp`

**功能：**
- 使用 OpenGL 3.3 Core Profile 渲染三角形
- 支持多个三角形组（每组可以有不同颜色）
- 使用索引绘制优化性能

**主要方法：**

```cpp
bool initialize(QOpenGLFunctions_3_3_Core* gl);  // 初始化
void render(const float* cameraMat);              // 渲染
void cleanup();                                   // 清理资源
void updateData(const std::vector<TriangleData>& triangleDatas);  // 更新数据
```

**数据结构 (TriangleData)：**

```cpp
struct TriangleData
{
    long long id;                       // 唯一标识
    std::vector<float> vVerts;          // 顶点数据 (x, y, z)
    std::vector<unsigned int> vIndices; // 索引数据
    Brush brush;                        // 渲染属性（颜色、深度）
};
```

---

### 3. FakeTriangleHelper (辅助工具)

**文件位置：**
- `RenderApp/FakeData/FakeTriangleHelper.h`

**功能：**
- 将 FakeTriangle 生成的数据转换为 TriangleData 格式
- 简化数据转换流程

**辅助函数：**

```cpp
// 转换单个 FakeTriangle
TriangleData convertToTriangleData(
    const FakeTriangle& fakeTriangle,
    long long id = 0,
    const Brush& brush = Brush{1.0f, 0.5f, 0.0f, 1.0f, 0.0f}
);

// 批量转换
std::vector<TriangleData> convertToTriangleDatas(
    const std::vector<FakeTriangle>& fakeTriangles,
    const std::vector<Brush>& brushes = {}
);
```

---

## 完整使用流程

### 基础用法

```cpp
#include "FakeData/FakeTriangle.h"
#include "FakeData/FakeTriangleHelper.h"
#include "Render/TriangleRenderer.h"

// 1. 初始化渲染器
QOpenGLFunctions_3_3_Core* gl = /* 获取 OpenGL 上下文 */;
GLRhi::TriangleRenderer renderer;
renderer.initialize(gl);

// 2. 生成多边形数据
GLRhi::FakeTriangle fakeTriangle;
fakeTriangle.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
fakeTriangle.generatePolygons(10, 3, 10, 0.5f);

// 3. 转换数据
GLRhi::Brush brush(1.0f, 0.5f, 0.0f, 1.0f, 0.0f);  // 橙色
GLRhi::TriangleData data = GLRhi::convertToTriangleData(fakeTriangle, 1, brush);

// 4. 上传到渲染器
renderer.updateData({data});

// 5. 渲染
float cameraMat[16] = { /* 相机矩阵 */ };
renderer.render(cameraMat);

// 6. 清理
renderer.cleanup();
```

### 多颜色多边形示例

```cpp
std::vector<GLRhi::TriangleData> triangleDatas;

// 生成红色凸多边形
GLRhi::FakeTriangle redTriangles;
redTriangles.generatePolygons(5, 4, 8, 0.2f);  // 20% 凹多边形
triangleDatas.push_back(GLRhi::convertToTriangleData(
    redTriangles, 1, GLRhi::Brush(1.0f, 0.0f, 0.0f, 1.0f, 0.0f)
));

// 生成绿色混合多边形
GLRhi::FakeTriangle greenTriangles;
greenTriangles.generatePolygons(5, 4, 8, 0.5f);  // 50% 凹多边形
triangleDatas.push_back(GLRhi::convertToTriangleData(
    greenTriangles, 2, GLRhi::Brush(0.0f, 1.0f, 0.0f, 1.0f, 0.1f)
));

// 生成蓝色凹多边形
GLRhi::FakeTriangle blueTriangles;
blueTriangles.generatePolygons(5, 4, 8, 0.8f);  // 80% 凹多边形
triangleDatas.push_back(GLRhi::convertToTriangleData(
    blueTriangles, 3, GLRhi::Brush(0.0f, 0.0f, 1.0f, 1.0f, 0.2f)
));

// 一次性渲染所有多边形
renderer.updateData(triangleDatas);
renderer.render(cameraMat);
```

---

## 参数说明

### FakeTriangle::generatePolygons 参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| polygonCount | size_t | 1 | 生成的多边形数量 |
| minVertices | size_t | 3 | 每个多边形的最小顶点数 |
| maxVertices | size_t | 20 | 每个多边形的最大顶点数 |
| concaveRatio | float | 0.5f | 凹多边形比例 (0.0=全凸, 1.0=全凹) |

### Brush 参数

```cpp
Brush(float r, float g, float b, float a, float depth)
```

| 参数 | 说明 | 范围 |
|------|------|------|
| r | 红色分量 | 0.0 - 1.0 |
| g | 绿色分量 | 0.0 - 1.0 |
| b | 蓝色分量 | 0.0 - 1.0 |
| a | 透明度 | 0.0 - 1.0 |
| depth | 深度值 | 0.0 - 1.0 |

---

## 渲染流程

```
┌─────────────────┐
│  FakeTriangle   │ 生成随机多边形
│  generatePolygons│
└────────┬────────┘
         │ 顶点 + 索引
         ▼
┌─────────────────┐
│     Earcut      │ 三角剖分算法
│  (自动调用)      │
└────────┬────────┘
         │ 剖分后的三角形
         ▼
┌─────────────────┐
│ FakeTriangleHelper│ 数据转换
│ convertToTriangleData│
└────────┬────────┘
         │ TriangleData
         ▼
┌─────────────────┐
│ TriangleRenderer│ OpenGL 渲染
│  updateData     │
│  render         │
└─────────────────┘
```

---

## 技术细节

### 多边形生成算法

**凸多边形：**
- 使用极坐标方法
- 在圆周上生成随机角度并排序
- 为每个角度分配随机半径

**凹多边形：**
- 基于凸多边形生成
- 随机选择部分顶点使用较小半径
- 创造"凹陷"效果

### 三角剖分

使用 [Mapbox Earcut](https://github.com/mapbox/earcut.hpp) 库：
- 高性能的多边形三角剖分算法
- 支持凸多边形和凹多边形
- 支持带孔的多边形（当前未使用）

### 渲染优化

- 使用 VAO/VBO/EBO 进行高效渲染
- 支持批量渲染多个三角形组
- 每个颜色组一次 DrawCall

---

## 依赖项

- **OpenGL 3.3 Core Profile**
- **Qt OpenGL** (QOpenGLFunctions_3_3_Core, QOpenGLShaderProgram)
- **Earcut.hpp** (`RenderApp/3rdpart/earcut.hpp`)
- **BaseTriangleShader** (GLSL 着色器)

---

## 注意事项

1. **坐标范围：** 使用 `setRange()` 设置生成范围，默认为 NDC 坐标 (-1 到 1)
2. **顶点数限制：** 建议 maxVertices 不超过 100
3. **内存管理：** updateData 会重新分配 GPU 缓冲区
4. **线程安全：** 生成和渲染应在同一 OpenGL 上下文线程

---

## 示例项目

完整示例代码请参考：
- `RenderApp/FakeData/TriangleRenderExample.cpp`

---

## 未来扩展

可能的改进方向：
- [ ] 支持带孔的多边形
- [ ] 添加纹理支持
- [ ] 实现实例化渲染
- [ ] 添加动画效果
- [ ] 支持 3D 多边形

---

## 相关类

参考 LineRenderer 的实现了解更多渲染器设计模式：
- `RenderEngine/include/Render/LineRenderer.h`
- `RenderEngine/src/Render/LineRenderer.cpp`

