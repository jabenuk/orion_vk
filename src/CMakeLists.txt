# ====================================================== #
#                 Orion CMake lists: SRC                 #
# ====================================================== #

#
# add source files

set(
    SRC
        "init.c"
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
        "${ORION_VENDOR_DIR}"
        "${CMAKE_CURRENT_LIST_DIR}"
)
