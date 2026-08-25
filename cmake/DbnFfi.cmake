#
# Acquire libdbn_c, the C interface to the Rust DBN library
#
# Every dbn release publishes a prebuilt archive for the platforms listed below, and
# that's what this uses by default: no Rust toolchain required. Other platforms, and
# builds against an unreleased dbn, go through Cargo instead.
#
# Sets:
#   dbn_c_target                 target to link against
#   dbn_c_installed_lib          static library to install alongside the client
#   dbn_c_lib_name               file name of the static library
#   dbn_c_native_static_libs     libraries the static library depends on
#   dbn_c_native_link_options    linker flags the static library depends on
#

set(dbn_version 0.68.0)

set(dbn_c_sha256_x86_64-unknown-linux-gnu
  29b24cf9b0b011f4353eb4f9965ba8e8cdb83f2b9e5535b6050d9004644b1750)
set(dbn_c_sha256_aarch64-unknown-linux-gnu
  ce78c36ed7642733d17b3d01912cdc2f28f84411c16145e0fe5ffbb5d722ccb4)
set(dbn_c_sha256_x86_64-apple-darwin
  410f05bec12719000fba9e622a44a0534bae25ac8bd2ab18b3dcf054312bbfd0)
set(dbn_c_sha256_aarch64-apple-darwin
  bee3deb1d6bcc18661e34a181530c55d42c21e1213d09151e3f4defdb5982cc3)
set(dbn_c_sha256_x86_64-pc-windows-msvc
  28fbeaa0341b2d036f7c9f6206327e1049d8af5f0de3d4f438204f4f6a06f22e)

#
# Determine which prebuilt archive fits the target platform, if any
#

set(dbn_c_triple "")
if(APPLE)
  list(LENGTH CMAKE_OSX_ARCHITECTURES dbn_c_macos_arch_count)
  if(dbn_c_macos_arch_count EQUAL 1)
    set(dbn_c_arch "${CMAKE_OSX_ARCHITECTURES}")
  elseif(dbn_c_macos_arch_count GREATER 1)
    # No single archive covers a universal binary
    set(dbn_c_arch "universal")
  else()
    set(dbn_c_arch "${CMAKE_SYSTEM_PROCESSOR}")
  endif()
  if(dbn_c_arch MATCHES "^(arm64|aarch64)$")
    set(dbn_c_triple aarch64-apple-darwin)
  elseif(dbn_c_arch STREQUAL "x86_64")
    set(dbn_c_triple x86_64-apple-darwin)
  endif()
elseif(WIN32)
  if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|arm64)$")
    set(dbn_c_triple x86_64-pc-windows-msvc)
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # The prebuilt Linux archives are built against glibc
  execute_process(
    COMMAND "${CMAKE_CXX_COMPILER}" -dumpmachine
    OUTPUT_VARIABLE dbn_c_compiler_triple
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if(dbn_c_compiler_triple MATCHES "gnu$")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
      set(dbn_c_triple x86_64-unknown-linux-gnu)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
      set(dbn_c_triple aarch64-unknown-linux-gnu)
    endif()
  endif()
endif()

if(MSVC)
  set(dbn_c_lib_name dbn_c.lib)
else()
  set(dbn_c_lib_name libdbn_c.a)
endif()

if(${PROJECT_NAME_UPPERCASE}_BUILD_DBN_FROM_SOURCE OR ${PROJECT_NAME_UPPERCASE}_DBN_SOURCE_DIR)
  set(dbn_c_from_source ON)
elseif(dbn_c_triple)
  set(dbn_c_from_source OFF)
else()
  message(STATUS "No prebuilt libdbn_c for this platform, building it with Cargo")
  set(dbn_c_from_source ON)
endif()

include(FetchContent)

if(NOT dbn_c_from_source)
  #
  # Prebuilt release archive
  #

  if(dbn_c_triple MATCHES "windows")
    set(dbn_c_archive_ext zip)
  else()
    set(dbn_c_archive_ext tar.gz)
  endif()

  FetchContent_Declare(
    dbn_c
    URL https://github.com/databento/dbn/releases/download/v${dbn_version}/libdbn_c-${dbn_version}-${dbn_c_triple}.${dbn_c_archive_ext}
    URL_HASH SHA256=${dbn_c_sha256_${dbn_c_triple}}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(dbn_c)

  # The archive records the libraries its static library expects to be linked with
  file(READ "${dbn_c_SOURCE_DIR}/native-static-libs.txt" dbn_c_native_static_libs)
  string(STRIP "${dbn_c_native_static_libs}" dbn_c_native_static_libs)
  separate_arguments(dbn_c_native_static_libs NATIVE_COMMAND "${dbn_c_native_static_libs}")

  # `rustc` mixes MSVC linker flags like `/defaultlib:msvcrt` in with the libraries.
  # CMake reads a leading `/` as a file path, so the flags go in link options instead
  set(dbn_c_native_link_options "")
  if(MSVC)
    set(dbn_c_libs "")
    foreach(dbn_c_item IN LISTS dbn_c_native_static_libs)
      if(dbn_c_item MATCHES "^/")
        list(APPEND dbn_c_native_link_options "${dbn_c_item}")
      else()
        list(APPEND dbn_c_libs "${dbn_c_item}")
      endif()
    endforeach()
    set(dbn_c_native_static_libs "${dbn_c_libs}")

    # Drop the CRT `rustc` names and leave it to whatever links the static library in.
    # Matches what corrosion does
    list(FILTER dbn_c_native_static_libs EXCLUDE REGEX "^msvcrtd?(\\.lib)?$")
    list(FILTER dbn_c_native_link_options EXCLUDE REGEX "^/defaultlib:msvcrtd?$")
  endif()

  add_library(dbn::dbn_c STATIC IMPORTED GLOBAL)
  set_target_properties(
    dbn::dbn_c
    PROPERTIES
    IMPORTED_LOCATION "${dbn_c_SOURCE_DIR}/${dbn_c_lib_name}"
    INTERFACE_LINK_LIBRARIES "${dbn_c_native_static_libs}"
    INTERFACE_LINK_OPTIONS "${dbn_c_native_link_options}"
  )
  set(dbn_c_include_dir "${dbn_c_SOURCE_DIR}/include")
  set(dbn_c_target dbn::dbn_c)
  set(dbn_c_installed_lib "${dbn_c_SOURCE_DIR}/${dbn_c_lib_name}")
  message(STATUS "Using prebuilt libdbn_c ${dbn_version} for ${dbn_c_triple}")
else()
  #
  # Build the crate with Cargo through corrosion
  #

  FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG v0.6.1
  )
  FetchContent_MakeAvailable(Corrosion)

  if(${PROJECT_NAME_UPPERCASE}_DBN_SOURCE_DIR)
    set(dbn_c_manifest "${${PROJECT_NAME_UPPERCASE}_DBN_SOURCE_DIR}/Cargo.toml")
    if(NOT EXISTS "${dbn_c_manifest}")
      message(FATAL_ERROR "No Cargo.toml in ${PROJECT_NAME_UPPERCASE}_DBN_SOURCE_DIR: ${dbn_c_manifest}")
    endif()
  else()
    FetchContent_Declare(
      dbn_src
      URL https://github.com/databento/dbn/archive/refs/tags/v${dbn_version}.tar.gz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(dbn_src)
    set(dbn_c_manifest "${dbn_src_SOURCE_DIR}/Cargo.toml")
  endif()

  set(dbn_c_include_dir "${CMAKE_CURRENT_BINARY_DIR}/dbn_c/include")
  file(MAKE_DIRECTORY "${dbn_c_include_dir}/dbn")

  corrosion_import_crate(MANIFEST_PATH "${dbn_c_manifest}" CRATES dbn-c)
  corrosion_set_env_vars(dbn_c DBN_C_HEADER_DIR=${dbn_c_include_dir}/dbn)
  set(dbn_c_target dbn_c-static)
  set(dbn_c_installed_lib "$<TARGET_FILE:dbn_c-static>")
  get_target_property(dbn_c_native_static_libs dbn_c-static INTERFACE_LINK_LIBRARIES)
  if(NOT dbn_c_native_static_libs)
    set(dbn_c_native_static_libs "")
  endif()
  set(dbn_c_native_link_options "")
  message(STATUS "Building libdbn_c from ${dbn_c_manifest}")
endif()

# Ignore compiler warnings in the generated header
set_target_properties(
  ${dbn_c_target}
  PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${dbn_c_include_dir}"
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${dbn_c_include_dir}"
)
