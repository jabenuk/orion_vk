# ====================================================== #
#                 Orion CMake lists: SRC                 #
# ====================================================== #

#
# add source files

set(SRC
    "headers/orion_errors.h"
    "headers/orion_funcs.h"
    "headers/orion_structs.h"

    "lib/callback.c"
    "lib/debug.c"
    "lib/init.c"

    "lib/vk_device.c"
    "lib/vk_ext.c"
)

#
# compile as desired lib type

if (BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${SRC})
else()
    add_library(${PROJECT_NAME} STATIC ${SRC})
endif()

#
# link to vulkan SDK

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

#
# include directories

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        "${ORION_INCLUDE_DIR}"
        "${ORION_CORE_VENDOR_DIR}"
        "${Vulkan_INCLUDE_DIRS}"
    PRIVATE
        "${ORION_SRC_DIR}/headers"
)

#
# add optimised flag if requested

if (ORION_RADOP)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "ORION_RADOP")
    message(WARNING "Building with radical optimisation (RADOP) -- debug output will be limited.")
endif()
