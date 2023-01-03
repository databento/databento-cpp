add_test(
  NAME cmake_fetch_content_configure
  COMMAND
    ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    ${CMAKE_CURRENT_SOURCE_DIR}/project
)
if(DEFINED OPENSSL_ROOT_DIR)
  set_tests_properties(
    cmake_fetch_content_configure
    PROPERTIES ENVIRONMENT "OPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}"
  )
endif()
add_test(
  NAME cmake_fetch_content_build
  COMMAND ${CMAKE_COMMAND} --build .
)
set_tests_properties(
  cmake_fetch_content_configure
  PROPERTIES FIXTURES_SETUP cmake_fetch_content
)
set_tests_properties(
  cmake_fetch_content_build
  PROPERTIES FIXTURES_REQUIRED cmake_fetch_content
)
