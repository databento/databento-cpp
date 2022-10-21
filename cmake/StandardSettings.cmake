#
# Project settings
#

option(DATABENTO_USE_EXTERNAL_JSON "Use an external JSON library" OFF)
option(${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_HTTPLIB "Use an external httplib library" OFF)

#
# Compiler options
#

option(${PROJECT_NAME_UPPERCASE}_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." ON)
option(${PROJECT_NAME_UPPERCASE}_FORCE_COLOR_OUTPUT "Always produce ANSI-colored output" OFF)

if(${PROJECT_NAME_UPPERCASE}_FORCE_COLOR_OUTPUT)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
  else()
    message(WARNING "Couldn't force color output with unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

#
# Unit testing
#

option(${PROJECT_NAME_UPPERCASE}_ENABLE_UNIT_TESTING "Enable unit tests for the projects (from the `test` subfolder)." ON)

option(${PROJECT_NAME_UPPERCASE}_USE_GTEST "Use the GoogleTest project for creating unit tests." ON)
option(${PROJECT_NAME_UPPERCASE}_USE_GOOGLE_MOCK "Use the GoogleMock project for extending the unit tests." OFF)

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
# Miscelanious options
#

# Generate compile_commands.json for clang based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(${PROJECT_NAME_UPPERCASE}_VERBOSE_OUTPUT "Enable verbose output, allowing for a better understanding of each step taken." ON)
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
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()
