include(FindPackageHandleStandardArgs)

find_library(ZSTD_LIBRARY NAMES zstd)
find_path(ZSTD_INCLUDE_DIR NAMES zstd.h)

#
# Detect version
#
if(ZSTD_INCLUDE_DIR)
  file(
    STRINGS "${ZSTD_INCLUDE_DIR}/zstd.h"
    version-file
    REGEX "^#define[ \t]ZSTD_VERSION_(MAJOR|MINOR|RELEASE).*$"
  )
  if(NOT version-file)
    message(AUTHOR_WARNING "ZSTD_INCLUDE_DIR found, but missing version info")
  endif()
  list(GET version-file 0 major-line)
  list(GET version-file 1 minor-line)
  list(GET version-file 2 patch-line)
  string(REGEX MATCH "[0-9]+$" ZSTD_VERSION_MAJOR ${major-line})
  string(REGEX MATCH "[0-9]+$" ZSTD_VERSION_MINOR ${minor-line})
  string(REGEX MATCH "[0-9]+$" ZSTD_VERSION_PATCH ${patch-line})
  set(ZSTD_VERSION ${ZSTD_VERSION_MAJOR}.${ZSTD_VERSION_MINOR}.${ZSTD_VERSION_PATCH} CACHE STRING "Zstd version")
endif()

find_package_handle_standard_args(
  Zstd
  REQUIRED_VARS ZSTD_LIBRARY ZSTD_INCLUDE_DIR
  VERSION_VAR ZSTD_VERSION
)

if(ZSTD_FOUND)
  mark_as_advanced(ZSTD_LIBRARY)
  mark_as_advanced(ZSTD_INCLUDE_DIR)
  mark_as_advanced(ZSTD_VERSION)
endif()

#
# Create namespaced target
#
if(ZSTD_FOUND AND NOT TARGET zstd::zstd)
  add_library(zstd::zstd UNKNOWN IMPORTED)
  set_target_properties(
    zstd::zstd
    PROPERTIES
      IMPORTED_LOCATION ${ZSTD_LIBRARY}
      # target_include_directories doesn't work with unknown imported libraries in older
      # cmake versions
      INTERFACE_INCLUDE_DIRECTORIES ${ZSTD_INCLUDE_DIR}
  )
endif()

# Check if the Conan-provided target exists
if(TARGET zstd::libzstd_static)
  # If the Conan target exists, use it
  set(ZSTD_TARGET zstd::libzstd_static)
elseif(TARGET zstd::zstd)
  # If the system-installed target exists, use it
  set(ZSTD_TARGET zstd::zstd)
else()
  # Error out if neither target is found
  message(FATAL_ERROR "Zstd target not found.")
endif()
