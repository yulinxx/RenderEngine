cmake_minimum_required(VERSION 3.10)

project(RenderEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt5 COMPONENTS OpenGL REQUIRED)

# 添加stb库支持
find_package(Stb REQUIRED)

# 分别收集不同目录下的源文件和头文件
file(GLOB_RECURSE RENDER_SOURCES
"src/Common/*.cpp"
"src/DataManager/*.cpp"
"src/FakeData/*.cpp"
"src/Render/*.cpp"
"src/Shader/*.cpp"
)

file(GLOB_RECURSE RENDER_HEADERS
"include/*.h"
"include/Common/*.h"
"include/DataManager/*.h"
"include/FakeData/*.h"
"include/Render/*.h"
"include/Shader/*.h"
)

# 为Visual Studio创建源代码过滤器
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${RENDER_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${RENDER_HEADERS})

# 生成DLL项目
add_library(${PROJECT_NAME} SHARED
    ${RENDER_SOURCES}
    ${RENDER_HEADERS}
)

# 设置DLL导出宏
target_compile_definitions(${PROJECT_NAME} PRIVATE "OPENGL_RENDER_DLL_EXPORTS")

# 添加头文件包含目录
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${Stb_INCLUDE_DIR}
)

# 链接必要的库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::OpenGL)

# 设置DLL的输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    DEBUG_POSTFIX "d"
)