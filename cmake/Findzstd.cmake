include(FindPackageHandleStandardArgs)

if(WIN32)
  find_library(ZSTD_SHARED_LIBRARY NAMES zstd)
else()
  set(_previous_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".dylib")
  find_library(ZSTD_SHARED_LIBRARY NAMES zstd)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
  find_library(ZSTD_STATIC_LIBRARY NAMES zstd zstd_static zstd-static)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_previous_suffixes})
endif()

find_path(ZSTD_INCLUDE_DIR NAMES zstd.h)

if(ZSTD_SHARED_LIBRARY)
  set(ZSTD_LIBRARY "${ZSTD_SHARED_LIBRARY}")
elseif(ZSTD_STATIC_LIBRARY)
  set(ZSTD_LIBRARY "${ZSTD_STATIC_LIBRARY}")
endif()

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
  zstd
  REQUIRED_VARS ZSTD_LIBRARY ZSTD_INCLUDE_DIR
  VERSION_VAR ZSTD_VERSION
)

if(ZSTD_FOUND)
  mark_as_advanced(ZSTD_LIBRARY)
  mark_as_advanced(ZSTD_INCLUDE_DIR)
  mark_as_advanced(ZSTD_VERSION)
else()
  # Error out if neither target is found
  message(FATAL_ERROR "zstd target not found.")
endif()

#
# Create namespaced target
#
if(NOT TARGET zstd::libzstd)
  if(ZSTD_SHARED_LIBRARY)
    if(WIN32)
      add_library(zstd::libzstd UNKNOWN IMPORTED)
    else()
      add_library(zstd::libzstd SHARED IMPORTED)
    endif()
    if (NOT TARGET zstd::libzstd_shared)
      add_library(zstd::libzstd_shared ALIAS zstd::libzstd)
    endif()
  else()
    add_library(zstd::libzstd STATIC IMPORTED)
    if (NOT TARGET zstd::libzstd_static)
      add_library(zstd::libzstd_static ALIAS zstd::libzstd)
    endif()
  endif()
  set_target_properties(
    zstd::libzstd
    PROPERTIES
      IMPORTED_LOCATION ${ZSTD_LIBRARY}
      # target_include_directories doesn't work with unknown imported libraries in older
      # cmake versions
      INTERFACE_INCLUDE_DIRECTORIES ${ZSTD_INCLUDE_DIR}
  )
endif()
