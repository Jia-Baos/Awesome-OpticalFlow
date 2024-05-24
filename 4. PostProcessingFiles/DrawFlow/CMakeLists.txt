cmake_minimum_required(VERSION 3.27)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_COMPILER "D:/MinGW-11.2.0/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "D:/MinGW-11.2.0/bin/g++.exe")

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

project(flo2png)

add_executable(flo2png color_flow.cpp flowIO.h flowIO.cpp colorcode.h colorcode.cpp)

set_target_properties(flo2png
		PROPERTIES
		CXX_STANDARD 20
		CXX_EXTENSIONS OFF
		CXX_STANDARD_REQUIRED ON
)

target_include_directories(flo2png
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/install/include
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imageLib/zib/include
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imageLib/pnglib/include
		)

target_link_directories(flo2png
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/install/lib
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imageLib/zlib/lib
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imageLib/pnglib/lib
)

target_link_libraries(flo2png 
		PUBLIC libimage.a libpng16.a libzlibstatic.a)