#----------------------------------------------------------------
# Generated CMake target import file for configuration "RELEASE".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "KF5::WidgetsAddons" for configuration "RELEASE"
set_property(TARGET KF5::WidgetsAddons APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(KF5::WidgetsAddons PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libKF5WidgetsAddons.a"
  )

list(APPEND _cmake_import_check_targets KF5::WidgetsAddons )
list(APPEND _cmake_import_check_files_for_KF5::WidgetsAddons "${_IMPORT_PREFIX}/lib/libKF5WidgetsAddons.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
