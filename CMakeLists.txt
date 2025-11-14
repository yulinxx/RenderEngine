cmake_minimum_required(VERSION 3.10)

# 使用vcpkg进行包管理
if(WIN32)
    # set(VCPKG_PATH C:/vcpkg/scripts/buildsystems/vcpkg.cmake)
    set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(UNIX AND NOT APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

set(CMAKE_TOOLCHAIN_FILE ${VCPKG_PATH})

# 设置项目名称和版本
project(RenderEngine VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# 根据不同操作系统设置Qt的CMake前缀路径
if(WIN32)
    list(APPEND CMAKE_PREFIX_PATH C:/Qt/5.15.2/msvc2019_64/lib/cmake)
elseif(UNIX AND NOT APPLE)
    list(APPEND CMAKE_PREFIX_PATH /opt/Qt/5.15.2/gcc_64/lib/cmake/)
elseif(APPLE)
    # 可在此处添加苹果系统的Qt路径设置
endif()

set(Qt_INSTALL_DIR "C:/Qt/5.15.2/msvc2019_64/")
set(Qt_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5")

# 设置全局输出目录，确保DLL和EXE在同一个目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 首先添加RenderEngine子目录（DLL），确保它在RenderApp之前构建
add_subdirectory(RenderEngine)

# 然后添加RenderApp子目录（EXE）
add_subdirectory(RenderApp)

message("--------- 项目配置完成 ---------")
message("- RenderEngine: 构建为DLL库")
message("- RenderApp: 构建为可执行文件并链接到RenderEngine DLL")
message("- 所有输出文件将位于: ${CMAKE_BINARY_DIR}/bin")
