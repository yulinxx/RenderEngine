# FakeData 命名空间移除说明

## 变更概述

已将 `RenderApp/FakeData` 文件夹下所有类从 `GLRhi` 命名空间中移除，使其成为全局命名空间的类。

## 变更原因

- FakeData 文件夹下的类是数据生成工具类，不属于渲染引擎的核心部分
- 移除命名空间可以简化使用，减少代码冗余
- 保持与项目其他数据生成工具的一致性

## 修改的文件列表

### 头文件 (.h)

1. ✅ `FakeDataBase.h` - 基类
2. ✅ `FakePolyLineData.h` - 多段线数据生成
3. ✅ `FakeTriangle.h` - 三角形数据生成
4. ✅ `FakeTriangleData.h` - 三角形数据（旧版）
5. ✅ `FakeTextureData.h` - 纹理数据生成
6. ✅ `FakeDataProvider.h` - 数据提供者
7. ✅ `InstanceTriangleFakeData.h` - 实例化三角形数据
8. ✅ `InstanceLineFakeData.h` - 实例化线段数据
9. ✅ `FakeTriangleHelper.h` - 三角形辅助函数

### 实现文件 (.cpp)

1. ✅ `FakeDataGenerator.cpp` - 数据生成器实现
2. ✅ `FakePolyLineData.cpp` - 多段线实现
3. ✅ `FakeTriangle.cpp` - 三角形实现
4. ✅ `FakeTriangleData.cpp` - 三角形数据实现
5. ✅ `FakeTextureData.cpp` - 纹理数据实现
6. ✅ `FakeDataProvider.cpp` - 数据提供者实现
7. ✅ `InstanceTriangleFakeData.cpp` - 实例化三角形实现
8. ✅ `InstanceLineFakeData.cpp` - 实例化线段实现
9. ✅ `TriangleRenderExample.cpp` - 示例代码

## 使用变更

### 之前的用法

```cpp
// 旧代码
GLRhi::FakeTriangle fakeTriangle;
fakeTriangle.generatePolygons(10, 3, 12, 0.5f);

GLRhi::FakePolyLineData lineData;
lineData.generateLines(5, 10, 100);
```

### 现在的用法

```cpp
// 新代码 - FakeData 类不再需要 GLRhi 前缀
FakeTriangle fakeTriangle;
fakeTriangle.generatePolygons(10, 3, 12, 0.5f);

FakePolyLineData lineData;
lineData.generateLines(5, 10, 100);
```

### 与渲染器交互

渲染相关的类型仍然在 `GLRhi` 命名空间中：

```cpp
// FakeData 类不需要命名空间
FakeTriangle fakeTriangle;
fakeTriangle.generatePolygons(20, 3, 12, 0.5f);

// 但渲染器和数据结构仍在 GLRhi 命名空间
GLRhi::TriangleRenderer renderer;
GLRhi::Brush brush(0.8f, 0.4f, 0.1f, 1.0f, 0.0f);
GLRhi::TriangleData data = convertToTriangleData(fakeTriangle, 1, brush);

renderer.initialize(gl);
renderer.updateData({data});
```

## 命名空间规则总结

### 不使用命名空间（全局）
- `FakeDataBase` 及其所有派生类
- `FakeTriangle`
- `FakePolyLineData`
- `FakeTriangleData`
- `FakeTextureData`
- `FakeDataProvider`
- `InstanceTriangleFakeData`
- `InstanceLineFakeData`
- 辅助函数（如 `convertToTriangleData`）

### 仍使用 GLRhi 命名空间
- 所有渲染器类（`TriangleRenderer`, `LineRenderer` 等）
- 渲染数据结构（`TriangleData`, `PolylineData`, `TextureData` 等）
- 渲染相关类型（`Brush`, `Color` 等）
- VBO 管理器（`PolylinesVboManager` 等）

## 迁移指南

如果你的代码中使用了 `GLRhi::FakeXXX` 类，需要进行以下修改：

### 1. 移除 FakeData 类的命名空间前缀

```cpp
// 修改前
GLRhi::FakeTriangle triangle;
GLRhi::FakePolyLineData lineData;
GLRhi::FakeDataProvider provider;

// 修改后
FakeTriangle triangle;
FakePolyLineData lineData;
FakeDataProvider provider;
```

### 2. 保留渲染相关类型的命名空间

```cpp
// 这些仍然需要 GLRhi 前缀
GLRhi::TriangleRenderer renderer;
GLRhi::TriangleData data;
GLRhi::Brush brush;
GLRhi::PolylineData polyline;
```

### 3. 更新 using 声明

```cpp
// 修改前
using GLRhi::FakeTriangle;
using GLRhi::FakePolyLineData;

// 修改后 - 不再需要 using 声明，直接使用
// FakeTriangle 和 FakePolyLineData 已经在全局命名空间
```

## 编译检查

所有修改已通过编译检查，无 linter 错误。

## 兼容性说明

这是一个**破坏性变更**。如果你的代码中使用了 `GLRhi::FakeXXX` 类，需要按照上述迁移指南进行修改。

建议使用全局搜索替换：
- 搜索：`GLRhi::Fake`
- 替换为：`Fake`

但要注意不要误替换渲染相关的类型。

## 测试建议

修改后请测试以下功能：
1. ✅ FakeTriangle 数据生成
2. ✅ FakePolyLineData 数据生成
3. ✅ 数据转换（convertToTriangleData 等）
4. ✅ 与渲染器的交互
5. ✅ FakeDataProvider 的所有方法

---

**修改日期：** 2025-12-10
**影响范围：** RenderApp/FakeData 文件夹下所有文件
**状态：** ✅ 已完成，无编译错误

