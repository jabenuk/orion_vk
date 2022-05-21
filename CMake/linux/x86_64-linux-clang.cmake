# ====================================================== #
#                  CMake toolchain file                  #
# -------------------------------------------------------#
# Compiled with Clang                                    #
# For x86_64 (64-bit) machines                           #
# Running Linux                                          #
# ====================================================== #

set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "-m64")

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-m64")
