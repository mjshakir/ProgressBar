#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ProgressBar::ProgressBar" for configuration ""
set_property(TARGET ProgressBar::ProgressBar APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(ProgressBar::ProgressBar PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_NOCONFIG "fmt::fmt;TBB::tbb"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libProgressBar.so"
  IMPORTED_SONAME_NOCONFIG "libProgressBar.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS ProgressBar::ProgressBar )
list(APPEND _IMPORT_CHECK_FILES_FOR_ProgressBar::ProgressBar "${_IMPORT_PREFIX}/lib/libProgressBar.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
