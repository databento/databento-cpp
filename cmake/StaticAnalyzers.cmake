if(${PROJECT_NAME_UPPERCASE}_ENABLE_CLANG_TIDY)
  find_program(CLANGTIDY NAMES clang-tidy clang-tidy-15 clang-tidy-14 clang-tidy-13)
  if(CLANGTIDY)
    set(
      CMAKE_CXX_CLANG_TIDY ${CLANGTIDY}
      -extra-arg=-Wno-unknown-warning-option
    )
    message(STATUS "Clang-Tidy finished setting up.")
  else()
    message(SEND_ERROR "Clang-Tidy requested but executable not found.")
  endif()
endif()

if(${PROJECT_NAME_UPPERCASE}_ENABLE_CPPCHECK)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    set(
      CMAKE_CXX_CPPCHECK ${CPPCHECK}
      --enable=all
      --suppress=missingIncludeSystem
      --suppress=preprocessorErrorDirective
      --suppress=unusedFunction
      --suppress=unmatchedSuppression
      --inline-suppr
      --relative-paths=${CMAKE_SOURCE_DIR}
      --quiet
      -I${CMAKE_SOURCE_DIR}/include
      -I${CMAKE_SOURCE_DIR}/src
      # Ignore tests
      -i${CMAKE_SOURCE_DIR}/test
      # Ignore third-party dependencies
      -i${CMAKE_BINARY_DIR}
    )
    message(STATUS "Cppcheck finished setting up.")
  else()
    message(SEND_ERROR "Cppcheck requested but executable not found.")
  endif()
endif()
