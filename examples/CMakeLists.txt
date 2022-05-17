# ====================================================== #
#               Orion CMake lists: EXAMPLES              #
# ====================================================== #

#
# helper function

function(add_orion_example)
# parse arguments
cmake_parse_arguments(
    ARG
    ""
    "NAME"
    "SRC"
    ${ARGN}
)

# create executable and add desired source
add_executable(${ARG_NAME} ${ORION_EXAMPLES_DIR}${ARG_SRC})

# link to required libraries (including Orion)
target_link_libraries(${ARG_NAME} ${PROJECT_NAME} glfw)
target_include_directories(${ARG_NAME} PUBLIC "${ORION_DEVEL_VENDOR_DIR}")
endfunction()

#
# add examples

# / / / / / / / /
# /     TBA     /
# / / / / / / / /
