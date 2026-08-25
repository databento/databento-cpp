# The project is configured in a nested directory because the client's own install
# script recurses into this one, and a project configured here would overwrite it
set(nested_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/nested)

add_test(
  NAME cmake_fetch_content_configure
  COMMAND
    ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -S ${CMAKE_CURRENT_SOURCE_DIR}/project
    -B ${nested_binary_dir}
)
if(DEFINED OPENSSL_ROOT_DIR)
  set_tests_properties(
    cmake_fetch_content_configure
    PROPERTIES ENVIRONMENT "OPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}"
  )
endif()
add_test(
  NAME cmake_fetch_content_build
  COMMAND ${CMAKE_COMMAND} --build ${nested_binary_dir}
)
set_tests_properties(
  cmake_fetch_content_configure
  PROPERTIES FIXTURES_SETUP cmake_fetch_content
)
set_tests_properties(
  cmake_fetch_content_build
  PROPERTIES FIXTURES_REQUIRED cmake_fetch_content
)
