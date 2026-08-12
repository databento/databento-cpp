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
#

set(dbn_version 0.67.0)

set(dbn_c_sha256_x86_64-unknown-linux-gnu
  d914299c51a222c3ba24da064f040873747d7fef2722cf795fa9c7a6390a8398)
set(dbn_c_sha256_aarch64-unknown-linux-gnu
  ad53ec377d35d7faebd3256f0fc4e176bc68f0f94297009f1c04b31b44ccfb1f)
set(dbn_c_sha256_x86_64-apple-darwin
  27950903196fc113576905a22c5ef64baa4982bad4196c1779d4fda8cab65a37)
set(dbn_c_sha256_aarch64-apple-darwin
  c72b0b03a70f51e8551ce676a4711b4ea6588f1856d6bef16f47aebcc4e20efa)
set(dbn_c_sha256_x86_64-pc-windows-msvc
  8c0eeb5a60c40307542e41ef5e83000f71c4f4964bddb045aa81a3ccf6cbbeb7)

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

  add_library(dbn::dbn_c STATIC IMPORTED GLOBAL)
  set_target_properties(
    dbn::dbn_c
    PROPERTIES
    IMPORTED_LOCATION "${dbn_c_SOURCE_DIR}/${dbn_c_lib_name}"
    INTERFACE_LINK_LIBRARIES "${dbn_c_native_static_libs}"
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
  message(STATUS "Building libdbn_c from ${dbn_c_manifest}")
endif()

# Ignore compiler warnings in the generated header
set_target_properties(
  ${dbn_c_target}
  PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${dbn_c_include_dir}"
  INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${dbn_c_include_dir}"
)
