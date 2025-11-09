cmake_minimum_required(VERSION 4.1)

##################################################################
# set(VCPKG_DIR "D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake")
#set(VCPKG_DIR "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
# set(Qt_INSTALL_DIR "C:/Qt/5.15.2/msvc2019_64/")
# set(Qt_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5")
# set(Qt_DIR "C:/Qt/5.15.2/msvc2019_64/")

#set(OUT_CMAKE_MSG  TRUE)
set(OUT_CMAKE_MSG FALSE)


project(RenderEngine LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(Qt5_DIR ${Qt_DIR})

find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)

# 收集所有源文件
file(GLOB SOURCES 
    "src/*.cpp"
    "src/Common/*.cpp"
    "src/Shader/*.cpp"
    "src/Widget/*.cpp"
    "src/Render/*.cpp"
    "src/FakeData/*.cpp"
)

add_executable(${PROJECT_NAME}
  ${SOURCES}
)

# 添加stb库支持
find_package(Stb REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::OpenGL)
target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/RenderEngine/src/)
message("Include Dir ${CMAKE_SOURCE_DIR}/RenderEngine/src/ ")


# 添加 Windows 平台部署 Qt 依赖库的命令
if(WIN32)
    # Windows 特定设置
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${Qt_INSTALL_DIR}/bin/windeployqt.exe" "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt libraries for Windows"
    )
endif()