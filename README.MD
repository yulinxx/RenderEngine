# C++ 测试项目

## 项目概述
RenderEngine 是一个基于 OpenGL 的 C++ 渲染引擎项目，主要用于图形渲染和 CAD 应用程序开发。该项目采用模块化架构，分为核心渲染引擎库和应用程序两大部分。

## 项目结构
### 1. RenderEngine
核心渲染引擎库，提供各种渲染功能和组件：

- Common : 包含基础组件如相机控制、颜色处理、纹理加载等
- Render : 实现多种渲染器，如线条渲染器、三角形渲染器、图像渲染器等
- Shader : 各种着色器实现，支持不同类型的渲染效果
- FakeData : 提供测试数据生成功能，用于渲染测试
### 2. RenderApp
使用渲染引擎的应用程序：

- Widget : 包含 UI 组件，如 RenderWidget（渲染控件）、ViewWrapper（视图包装器）等
- main.cpp : 应用程序入口点
## 核心功能
1. 多种渲染器支持 ：
   - 线条渲染（LineRenderer、LineBRenderer、InstanceLineRenderer）
   - 三角形渲染（TriangleRenderer、InstanceTriangleRenderer）
   - 图像渲染（ImageRenderer、TextureRenderer、InstanceTextureRenderer）
   - 棋盘格背景渲染（CheckerboardRenderer）
2. 交互功能 ：
   
   - 相机控制（平移、缩放）
   - 鼠标事件处理
   - 键盘事件处理
3. 渲染设置 ：
   
   - 背景颜色设置
   - 抗锯齿控制
   - 线框模式支持
4. 数据管理 ：
   
   - 渲染数据缓冲区更新
   - 实例化数据支持
   - 测试数据生成
5. 其他功能 ：
   
   - 截图功能
   - 视图范围控制
   - OpenGL 错误检查
## 技术栈
- 编程语言 : C++17
- 图形 API : OpenGL 3.3 Core Profile
- GUI 框架 : Qt5
- 构建系统 : CMake
- 依赖管理 : VCPKG
- 主要依赖库 : OpenGL、Qt、Boost、Eigen3、stb 等

## 环境配置
使用VCPKG安装依赖库：
```bash
vcpkg install boost eigen3 stb opencv vtk osg pybind11 glm glfw glad breakpad protobuf gtest glog spdlog opencascade:x64-windows
```

## 库分类说明

### 1. 通用库
- **Boost**：一系列高质量的C++库
  - boost-accumulators
  - boost-algorithm
  - boost-filesystem
- **C++标准库扩展**
  - fmt（格式化库)
  - spdlog（日志库)
  - nlohmann-json（JSON处理库)
- **跨平台库**
  - sqlite3（轻量级数据库)
  - zlib（压缩库)
  - openssl（加密库)

### 2. 图形和图像处理库
- OpenGL：标准的图形渲染库
- OpenCV：计算机视觉库
- FreeImage：图像格式支持库
- 图像编解码库：
  - libjpeg-turbo
  - libpng
  - libwebp
- OpenEXR：高动态范围（HDR)图像处理

### 3. 数学和科学计算库
- Eigen3：线性代数库
- CGAL：计算几何算法库
- Ceres Solver：非线性优化库
- GMP：高精度算术库
- MPFR：多精度浮点数计算库

### 4. 网络和通信库
- CURL：网络数据传输库
- Boost.Beast：基于Boost.Asio的网络编程库
- Poco：现代C++类库

### 5. 数据处理和解析库
- JSON处理：
  - JSON-C
  - JsonCpp
- XML解析：
  - libxml2
  - expat
- 数据存储：
  - SQLite3
  - Protobuf

### 6. 第三方工具和框架
- FFmpeg：音视频编解码
- 字体处理：
  - FreeType
  - HarfBuzz
- 3D图形：
  - OpenSceneGraph
- Web服务器：
  - CivetWeb

### 7. 其他库
- 压缩算法：
  - Brotli
  - Zstd
  - LZ4
- 日志系统：
  - Log4Cplus
  - GLog
- 地理处理：
  - Proj
  - GDAL

## 贝塞尔曲线处理库推荐

1. [GameFusion/Bezier](https://github.com/GameFusion/Bezier)
   - 超轻量级贝塞尔曲线库
   - 支持多种编程语言
   - 适用于3D动画、游戏开发

2. [RellyLiu/bezier](https://github.com/RellyLiu/bezier)
   - 单头文件C++11库
   - 专注于二维贝塞尔曲线

3. [romb-technologies/Bezier](https://github.com/romb-technologies/Bezier)
   - 支持任意阶数贝塞尔曲线
   - 提供曲线分割、交点计算等功能

4. [oysteinmyrmo/bezier](https://github.com/oysteinmyrmo/bezier)
   - 单头文件C++11库
   - 支持曲线创建、分割等操作

## 其它库
1. [三角剖分：earcut](https://github.com/mapbox/earcut.hpp)
  - earcut.hpp 是一个高效、仅头文件的C++多边形三角剖分库，由 Mapbox 开发并开源
  - 基于改进的耳切算法（Ear Clipping Algorithm)，通过 Z-Order 曲线哈希优化，能够处理复杂的几何形状，包括带孔洞的多边形、扭曲的多边形、退化情况以及自交.
