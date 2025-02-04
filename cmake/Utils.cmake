#
# Print a message only if the `VERBOSE_OUTPUT` option is on
#

function(verbose_message content)
  if(${PROJECT_NAME_UPPERCASE}_VERBOSE_OUTPUT)
    # message(VERBOSE ...) was added in cmake 3.15
    message(STATUS ${content})
  endif()
endfunction()

#
# Add a target for formating the project using `clang-format` (i.e: cmake --build build --target clang-format)
#

function(add_clang_format_target)
  if(NOT ${PROJECT_NAME_UPPERCASE}_CLANG_FORMAT_BINARY)
    find_program(${PROJECT_NAME_UPPERCASE}_CLANG_FORMAT_BINARY clang-format)
  endif()

  if(${PROJECT_NAME_UPPERCASE}_CLANG_FORMAT_BINARY)
    if(${PROJECT_NAME_UPPERCASE}_BUILD_EXECUTABLE)
      add_custom_target(clang-format
        COMMAND ${${PROJECT_NAME_UPPERCASE}_CLANG_FORMAT_BINARY}
        -i ${exe_sources} ${headers}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    else()
      add_custom_target(clang-format
        COMMAND ${${PROJECT_NAME_UPPERCASE}_CLANG_FORMAT_BINARY}
        -i ${sources} ${headers}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    endif()
    message(STATUS "Format the project using the `clang-format` target (i.e: cmake --build build --target clang-format).")
  endif()
endfunction()

#
# Make a target a system target so compiler warnings are ignored for its headers
#

macro(add_system_include_property NAME)
  get_target_property(${NAME}_IID ${NAME} INTERFACE_INCLUDE_DIRECTORIES)
  set_target_properties(${NAME} PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${${NAME}_IID}")
endmacro()

#
# Add an example target
#

function(add_example_target name file)
  add_executable(${name} ${file})
  target_link_libraries(
    ${name}
    PRIVATE
      databento::databento
  )
  target_compile_features(${name} PUBLIC cxx_std_17)
  set_target_warnings(${name})
endfunction()
