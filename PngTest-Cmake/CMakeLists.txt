cmake_minimum_required(VERSION 3.27)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_C_COMPILER "D:/MinGW-11.2.0/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "D:/MinGW-11.2.0/bin/g++.exe")

project(main)

aux_source_directory(. main_files)
add_executable(main ${main_files})

set_target_properties(main
		PROPERTIES
		CXX_STANDARD 20
		CXX_EXTENSIONS OFF
		CXX_STANDARD_REQUIRED ON
)

# 指定目标所需头文件的路径
target_include_directories(main
PUBLIC ${PROJECT_SOURCE_DIR}/zib/include
PUBLIC ${PROJECT_SOURCE_DIR}/pnglib/include)

# 指定目标所需库文件的路径
target_link_directories(main
PUBLIC ${PROJECT_SOURCE_DIR}/zlib/lib
PUBLIC ${PROJECT_SOURCE_DIR}/pnglib/lib)

# 链接静态库需要注意顺序：一般为从main的依赖往前倒推
target_link_libraries(main libpng16.a libzlibstatic.a)
