#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ynet::ynet" for configuration ""
set_property(TARGET ynet::ynet APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(ynet::ynet PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libynet.a"
  )

list(APPEND _cmake_import_check_targets ynet::ynet )
list(APPEND _cmake_import_check_files_for_ynet::ynet "${_IMPORT_PREFIX}/lib/libynet.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
