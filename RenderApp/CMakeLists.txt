cmake_minimum_required(VERSION 3.10)

project(GLRenderEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# 分别收集主程序和Widget目录下的源文件和头文件
file(GLOB MAIN_FILES
    "main.cpp"
)

file(GLOB WIDGET_SOURCES
    "Widget/*.cpp"
)

file(GLOB WIDGET_HEADERS
    "Widget/*.h"
)

# 合并所有文件
set(APP_SOURCES
    ${MAIN_FILES}
    ${WIDGET_SOURCES}
    ${WIDGET_HEADERS}
)

# 为Visual Studio创建源代码过滤器
source_group("Main" FILES ${MAIN_FILES})
source_group("Widget" FILES ${WIDGET_SOURCES} ${WIDGET_HEADERS})

# 生成可执行文件
add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
)

# 添加头文件包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/RenderEngine/include
)

# 添加RenderEngine的库目录
target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/lib
)

# 确保RenderEngine库在当前项目之前构建
add_dependencies(${PROJECT_NAME} RenderEngine)

# 链接DLL和Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    RenderEngine
)

# 设置可执行文件的输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    WIN32_EXECUTABLE TRUE
)

# 添加 Windows 平台部署 Qt 依赖库的命令
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${Qt_INSTALL_DIR}/bin/windeployqt.exe" "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt libraries for Windows"
    )
endif()