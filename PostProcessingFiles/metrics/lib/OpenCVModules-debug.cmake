#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "opencv_world" for configuration "Debug"
set_property(TARGET opencv_world APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_world PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x64/vc17/lib/opencv_world460d.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x64/vc17/bin/opencv_world460d.dll"
  )

list(APPEND _cmake_import_check_targets opencv_world )
list(APPEND _cmake_import_check_files_for_opencv_world "${_IMPORT_PREFIX}/x64/vc17/lib/opencv_world460d.lib" "${_IMPORT_PREFIX}/x64/vc17/bin/opencv_world460d.dll" )

# Import target "opencv_img_hash" for configuration "Debug"
set_property(TARGET opencv_img_hash APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(opencv_img_hash PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/x64/vc17/lib/opencv_img_hash460d.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/x64/vc17/bin/opencv_img_hash460d.dll"
  )

list(APPEND _cmake_import_check_targets opencv_img_hash )
list(APPEND _cmake_import_check_files_for_opencv_img_hash "${_IMPORT_PREFIX}/x64/vc17/lib/opencv_img_hash460d.lib" "${_IMPORT_PREFIX}/x64/vc17/bin/opencv_img_hash460d.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
