# ====================================================== #
#                 Orion CMake lists: SRC                 #
# ====================================================== #

#
# add source files

set(
    SRC
        "error.c"
        "init.c"
        "internal.h"
)

#
# compile as desired lib type

if (BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${SRC})
else()
    add_library(${PROJECT_NAME} STATIC ${SRC})
endif()

#
# include directories

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        "${ORION_INCLUDE_DIR}"
        "${ORION_CORE_VENDOR_DIR}"
    PRIVATE
        "${ORION_SRC_DIR}"
)
