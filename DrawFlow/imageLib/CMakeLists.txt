cmake_minimum_required(VERSION 3.27)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_COMPILER "D:/MinGW-11.2.0/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "D:/MinGW-11.2.0/bin/g++.exe")
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install Dir: ${CMAKE_INSTALL_PREFIX}")

project(main)

aux_source_directory(. func_files)

add_library(image_static STATIC ${func_files})
add_library(image_shared SHARED ${func_files})

set_target_properties(image_static image_shared
		PROPERTIES
		CXX_STANDARD 20
		CXX_EXTENSIONS OFF
		CXX_STANDARD_REQUIRED ON
)

set_target_properties(image_static PROPERTIES OUTPUT_NAME "image")
set_target_properties(image_shared PROPERTIES OUTPUT_NAME "image")

target_include_directories(image_static
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/zib/include
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pnglib/include
		)
target_include_directories(image_shared
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/zib/include
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pnglib/include
		)

target_link_directories(image_static
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/zlib/lib
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pnglib/lib
)
target_link_directories(image_shared
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/zlib/lib
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pnglib/lib
)

target_link_libraries(image_static 
		PUBLIC libpng16.a libzlibstatic.a)
target_link_libraries(image_shared 
		PUBLIC libpng16.dll libzlib.dll)

install(TARGETS image_static image_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "CMakeLists.txt" EXCLUDE)
