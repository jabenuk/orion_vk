# ====================================================== #
#                 Orion CMake lists: ROOT                #
# ====================================================== #

#
# minimum CMake version 3.10 required

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

#
# project declaration

project(orion LANGUAGES C)

#
# use full C99

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

#
# internal path variables

set(ORION_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src/")
set(ORION_TESTS_DIR "${CMAKE_CURRENT_LIST_DIR}/tests/")
set(ORION_EXAMPLES_DIR "${CMAKE_CURRENT_LIST_DIR}/examples/")
set(ORION_DOCS_DIR "${CMAKE_CURRENT_LIST_DIR}/docs/")
set(ORION_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include/")

set(ORION_CORE_VENDOR_DIR "${CMAKE_CURRENT_LIST_DIR}/deps/core/")
set(ORION_DEVEL_VENDOR_DIR "${CMAKE_CURRENT_LIST_DIR}/deps/devel/") # dependencies for development (tests + examples )

#
# user options

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

option(ORION_BUILD_CORE "Build the core Orion library" ON)
option(ORION_BUILD_TESTS "Build Orion test executables" OFF)
option(ORION_BUILD_EXAMPLES "Build Orion usage examples" OFF)
option(ORION_GEN_DOCS "Generate HTML Orion documentation" ON)

option(ORION_RADOP "Orion radical optimisations" OFF)

#
# subdirectories

if (ORION_BUILD_CORE)
    add_subdirectory("${ORION_SRC_DIR}")
endif()

if (ORION_BUILD_TESTS)
    add_subdirectory("${ORION_TESTS_DIR}")
endif()

if (ORION_BUILD_EXAMPLES)
    add_subdirectory("${ORION_EXAMPLES_DIR}")
endif()

if (ORION_GEN_DOCS)
    add_subdirectory("${ORION_DOCS_DIR}")
endif()

#
# compile devel-vendors if necessary

if (ORION_BUILD_TESTS OR ORION_BUILD_EXAMPLES)
    #
    # glfw

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)        # no docs
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)       # no tests
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)    # no examples

    add_subdirectory("${ORION_DEVEL_VENDOR_DIR}/glfw/")
endif()
