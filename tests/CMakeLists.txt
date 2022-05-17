# ====================================================== #
#                Orion CMake lists: TESTS                #
# ====================================================== #

#
# helper function

function(add_orion_test)
    # parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "NAME"
        "SRC"
        ${ARGN}
    )

    # create executable and add desired source
    add_executable(${ARG_NAME} ${ORION_TESTS_DIR}${ARG_SRC})

    # link to required libraries (including Orion)
    target_link_libraries(${ARG_NAME} ${PROJECT_NAME} glfw)
    target_include_directories(${ARG_NAME} PUBLIC "${ORION_DEVEL_VENDOR_DIR}")
endfunction()

#
# add tests

add_orion_test(NAME "main" SRC "main.c")
