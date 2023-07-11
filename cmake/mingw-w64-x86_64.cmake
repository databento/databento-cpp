set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

# Cross compilers to use for C and C++
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
# Compiler for Windows resource files
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# Target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# Modify default behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
