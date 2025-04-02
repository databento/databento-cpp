# Install locally
add_test(
  NAME cmake_import_install
  COMMAND
    ${CMAKE_COMMAND}
    --install ${CMAKE_BINARY_DIR}
    --prefix ${CMAKE_CURRENT_BINARY_DIR}/pkg
)
# Configure test project
add_test(
  NAME cmake_import_configure
  COMMAND
    ${CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -Ddatabento_DIR=${CMAKE_CURRENT_BINARY_DIR}/pkg/${CMAKE_INSTALL_LIBDIR}/cmake/databento
    -Dnlohmann_json_DIR=${nlohmann_json_BINARY_DIR}
    -Ddate_DIR=${CMAKE_CURRENT_BINARY_DIR}/pkg/${CMAKE_INSTALL_LIBDIR}/cmake/date
    -Dhttplib_DIR=${CMAKE_CURRENT_BINARY_DIR}/pkg/${CMAKE_INSTALL_LIBDIR}/cmake/httplib
    ${CMAKE_CURRENT_SOURCE_DIR}/project
)
# Build test project
add_test(
  NAME cmake_import_build
  COMMAND ${CMAKE_COMMAND} --build .
)
set_tests_properties(
  cmake_import_install
  PROPERTIES FIXTURES_SETUP cmake_import
)
set_tests_properties(
  cmake_import_configure
  PROPERTIES FIXTURES_SETUP cmake_import
)
set_tests_properties(
  cmake_import_build
  PROPERTIES FIXTURES_REQUIRED cmake_import
)
