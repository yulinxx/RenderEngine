cmake_minimum_required(VERSION 3.10)

# 使用vcpkg进行包管理
if(WIN32)
    set(VCPKG_PATH D:/Install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(UNIX AND NOT APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg/scripts/buildsystems/vcpkg.cmake)
elseif(APPLE)
    set(VCPKG_PATH /home/x/install/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

set(CMAKE_TOOLCHAIN_FILE ${VCPKG_PATH})

# set(VCPKG_TARGET_TRIPLET x64-windows-static)

project(RenderEngine VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

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

# 查找所有子目录并排除某些目录
message("--------- subdir ----- ")

# 定义要排除的目录列表
set(EXCLUDE_DIRS ".vs" ".vscode" ".git" "out")

# 自定义宏,用于查找当前目录下的所有子目录
macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            list(FIND EXCLUDE_DIRS ${child} _index)
            if(_index EQUAL -1)
                list(APPEND dirlist ${child})
            endif()
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

# 查找当前文件夹下的子目录
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

# 设置要排除的子目录
set(ExclusionList ".git" ".vscode" "build")

# 遍历子目录,将排除的子目剔除后,添加至cmake中
foreach(SUBDIR ${SUBDIRS})
    message("\n----------------------------" ${SUBDIR})
    set(ADD_DIR TRUE)
    foreach(exc ${ExclusionList})
        if("${SUBDIR}" STREQUAL "${exc}")
            message(" ${SUBDIR}   Equal  ${exc} Skip...")
            set(ADD_DIR FALSE)
            break()
        endif()
    endforeach()
    if(ADD_DIR)
        message("add subdir: " ${SUBDIR})
        add_subdirectory(${SUBDIR})
    endif()
endforeach()

message("--------- all done ----- ")
