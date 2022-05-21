# ====================================================== #
#                  CMake toolchain file                  #
# -------------------------------------------------------#
# Compiled with GCC                                      #
# For x86_64 (64-bit) machines                           #
# Running Linux                                          #
# ====================================================== #

set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-m64")

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-m64")
