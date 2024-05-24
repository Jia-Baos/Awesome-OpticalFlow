# 最低版本要求 cmake 构建工具小于3.20将报错
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# 指定生成模式
set(CMAKE_BUILD_TYPE "Debug")

# 设置编译器
set(CMAKE_C_COMPILER "D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/bin/Hostx64/x64/cl.exe")
set(CMAKE_CXX_COMPILER "D:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/bin/Hostx64/x64/cl.exe")

# 工程项目名称
project(main LANGUAGES CXX)

# 指定变量名称
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# 链接库文件
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/opencv2)
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 添加可执行文件
add_executable(main main.cpp)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # 编译Release版本仅连接Release库是不够的，还需要指定 --config Release    
    message(INFO "CMAKE_BUILD_TYPE: Release")
    target_link_libraries(main opencv_world460.lib)
else()
    message(INFO "CMAKE_BUILD_TYPE: Debug")
    target_link_libraries(main opencv_world460d.lib)
endif()