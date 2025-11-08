# C++ 测试项目

## 项目概述
本项目包含基础的C++测试代码，用于学习和验证各种C++库的使用.

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
