#
# Project settings
#

option(${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_JSON "Use an external JSON library" OFF)
option(${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_HTTPLIB "Use an external httplib library" OFF)
option(${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_GTEST "Use an external google test (gtest) library" ON)

#
# Compiler options
#

option(${PROJECT_NAME_UPPERCASE}_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." ${IS_MAIN_PROJECT})

#
# Unit testing
#

# Default to ON if main project, otherwise OFF
option(${PROJECT_NAME_UPPERCASE}_ENABLE_UNIT_TESTING "Enable unit tests for the projects (from the `test` subfolder)." OFF)
option(${PROJECT_NAME_UPPERCASE}_ENABLE_EXAMPLES "Enable building examples for the project." OFF)

#
# Static analyzers
#
# Currently supporting: Clang-Tidy, Cppcheck.

option(${PROJECT_NAME_UPPERCASE}_ENABLE_CLANG_TIDY "Enable static analysis with Clang-Tidy." OFF)
option(${PROJECT_NAME_UPPERCASE}_ENABLE_CPPCHECK "Enable static analysis with Cppcheck." OFF)

#
# Code coverage
#

option(${PROJECT_NAME_UPPERCASE}_ENABLE_CODE_COVERAGE "Enable code coverage through GCC." OFF)

#
# Sanitizers
#

option(${PROJECT_NAME_UPPERCASE}_ENABLE_ASAN "Enable address sanitizer." OFF)
option(${PROJECT_NAME_UPPERCASE}_ENABLE_TSAN "Enable thread sanitizer." OFF)
option(${PROJECT_NAME_UPPERCASE}_ENABLE_UBSAN "Enable undefined behavior sanitizer." OFF)

#
# Miscelanious options
#

# Generate compile_commands.json for clang based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ${IS_MAIN_PROJECT})

option(${PROJECT_NAME_UPPERCASE}_VERBOSE_OUTPUT "Enable verbose output, allowing for a better understanding of each step taken." ${IS_MAIN_PROJECT})
option(${PROJECT_NAME_UPPERCASE}_GENERATE_EXPORT_HEADER "Create a `project_export.h` file containing all exported symbols." OFF)

# Export all symbols when building a shared library
if(BUILD_SHARED_LIBS)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
endif()

option(${PROJECT_NAME_UPPERCASE}_ENABLE_LTO "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)." OFF)
if(${PROJECT_NAME_UPPERCASE}_ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT result OUTPUT output)
  if(result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  else()
    message(SEND_ERROR "IPO is not supported: ${output}.")
  endif()
endif()

option(${PROJECT_NAME_UPPERCASE}_ENABLE_CCACHE "Enable the usage of Ccache, in order to speed up rebuild times." ON)
if(${PROJECT_NAME_UPPERCASE}_ENABLE_CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    message(STATUS "Found and enabled ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  endif()
endif()
