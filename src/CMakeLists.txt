# ====================================================== #
#                 Orion CMake lists: SRC                 #
# ====================================================== #

#
# add source files

set(
    SRC
        "headers/orion_codes.h"
        "headers/orion_funcs.h"
        "headers/orion_helpers.h"
        "headers/orion_structs.h"

        "lib/callback.c"
        "lib/error.c"
        "lib/init.c"

        "vulkan/vk_debug.c"
        "vulkan/vk_extension.c"
        "vulkan/vk_init.c"
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

if (ORION_OPTIMISED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "ORION_OPTIMISED")
    message(WARNING "Building with ORION_OPTIMISED enabled -- debug output will be limited.")
endif()
