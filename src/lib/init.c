/* *************************************************************************************** */
/*                        ORION GRAPHICS LIBRARY AND RENDERING ENGINE                      */
/* *************************************************************************************** */
/* Copyright (c) 2022 Jack Bennett                                                         */
/* --------------------------------------------------------------------------------------- */
/* THE  SOFTWARE IS  PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND, EXPRESS  OR IMPLIED, */
/* INCLUDING  BUT  NOT  LIMITED  TO  THE  WARRANTIES  OF  MERCHANTABILITY,  FITNESS FOR  A */
/* PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN  NO EVENT SHALL  THE  AUTHORS  OR COPYRIGHT */
/* HOLDERS  BE  LIABLE  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF */
/* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR */
/* THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                              */
/* *************************************************************************************** */

/**
 * @file init.c
 * @author jack bennett
 * @brief Library initialisation, loading, and overall management
 *
 * @copyright Copyright (c) 2022
 *
 * This file defines functions involved with:
 *   - 'initialising' the library (i.e. creating a state object)
 *   - the creation of some core library structures
 *   - the overall termination of the library
 *   - configuring the library
 *
 */

#include "orion.h"
#include "orion_structs.h"
#include "orion_helpers.h"
#include "orion_funcs.h"
#include "orion_codes.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================

// initialisation of global internal variables
_ori_Library _orion = { NULL };



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        STATE                                                     *****
// ============================================================================

/**
 * @brief Create an Orion state object and return its handle.
 *
 * For the @c apiVersion parameter, you should use the @c VK_MAKE_API_VERSION macro defined in the Vulkan header.
 *
 * @param apiVersion the Vulkan version to use, as specified in the
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-coreversions-versionnumbers">Specification</a>.
 *
 * @return the resulting Orion state handle.
 *
 * @sa oriState
 * @sa oriDestroyState()
 *
 * @ingroup group_Meta
 *
 */
oriState *oriCreateState() {
    oriState *r = malloc(sizeof(oriState));

    // init to ensure everything (most importantly linked list heads) start at NULL
    memset(r, 0, sizeof(oriState));

#   ifdef __oridebug
        _ori_DebugLog("state object created into %p", r);
#   endif

    return r;
}

/**
 * @brief Destroy the specified Orion state.
 *
 * Calling this function clears all data stored in @c state and destroys all Orion objects registered to it.
 *
 * @param state handle to the state object to be destroyed.
 *
 * @sa oriState
 *
 * @ingroup group_Meta
 *
 */
void oriDestroyState(oriState *state) {
    if (!state) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return;
    }

    // first, we remove duplicates from any public arrays that store pointers
    // this is because trying to destroy the contents of the same address multiple times will lead to an exception.
    _ori_RemoveDArrayDuplicates(state->arrays.instances, state->arrays.instancesCount);
    _ori_RemoveDArrayDuplicates(state->arrays.logicalDevices, state->arrays.logicalDevicesCount);

    // load any necessary non-core Vulkan functions before the instances are destroyed
    // functions that rely on an instance are initialised as NULL before all instances are iterated through.
    PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = NULL;
    for (unsigned int i = 0; !DestroyDebugUtilsMessengerEXT && i < state->arrays.instancesCount; i++) {
        DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(*(state->arrays.instances[i]), "vkDestroyDebugUtilsMessengerEXT");
    }

    // destroy debug messengers
    // if the DestroyDebugUtilsMessengerEXT pfn is NULL, then the debug utils extension wasn't enabled and therefore no debug messengers could be
    // created by the user (at least not using Orion functions)
    if (DestroyDebugUtilsMessengerEXT) {
        for (unsigned int i = 0; i < state->arrays.debugMessengersCount; i++) {
            if (*state->arrays.debugMessengers[i].handle) {
                DestroyDebugUtilsMessengerEXT(*(state->arrays.debugMessengers[i].instance), *(state->arrays.debugMessengers[i].handle), _orion.callbacks.vulkanAllocators);
            }

#           ifdef __oridebug
                _ori_DebugLog("VkDebugUtilsMessengerEXT at %p was freed by state object at location %p", state->arrays.debugMessengers[i].handle, state);
#           endif
        }
    }
    // note: this may have to be done inside the above if statement, but it seems to work fine here
    _ori_FreeDArray(state->arrays.debugMessengers, state->arrays.debugMessengersCount);

    // destroy logical devices
    for (unsigned int i = 0; i < state->arrays.logicalDevicesCount; i++) {
        if (*state->arrays.logicalDevices[i]) {
            vkDestroyDevice(*state->arrays.logicalDevices[i], _orion.callbacks.vulkanAllocators);
        }

#       ifdef __oridebug
            _ori_DebugLog("VkDevice at %p was freed by state object at location %p", state->arrays.logicalDevices[i], state);
#       endif
    }
    _ori_FreeDArray(state->arrays.logicalDevices, state->arrays.logicalDevicesCount);

    // destroy instances
    for (unsigned int i = 0; i < state->arrays.instancesCount; i++) {
        if (*state->arrays.instances[i]) {
            vkDestroyInstance(*state->arrays.instances[i], _orion.callbacks.vulkanAllocators);
        }

#       ifdef __oridebug
            _ori_DebugLog("VkInstance at %p was freed by state object at location %p", state->arrays.instances[i], state);
#       endif
    }
    _ori_FreeDArray(state->arrays.instances, state->arrays.instancesCount)

    // free other dynamically allocated arrays
    _ori_FreeDArray(state->instanceCreateInfo.enabledExtensions, state->instanceCreateInfo.enabledExtCount);
    _ori_FreeDArray(state->instanceCreateInfo.enabledLayers, state->instanceCreateInfo.enabledLayerCount);

    // finally, free state pointer
#   ifdef __oridebug
        _ori_Notification("freed state at %p", state);
#   endif

    free(state);
    state = NULL;
}

/**
 * @brief Set application info for a state object.
 *
 * The @c apiVersion parameter of the application info is set in oriCreateState().
 *
 * It is not required to use this function (and hence VkApplicationInfo), but it is recommended so as to support what Vulkan calls 'driver optimisations'.
 * Whatever that means.
 *
 * @param state the state the object is to be registered into
 * @param ext equivalent to the @c pNext parameter in the Vulkan Specification (linked below): NULL or a pointer to a structure extending this structure.
 * @param apiVersion the Vulkan version to use, as specified in the
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-coreversions-versionnumbers">Specification</a>.
 * @param name NULL, or a string containing the name of the application.
 * @param version the version of the application.
 * @param engineName NULL, or a string containing the name of the engine used to create the application.
 * @param engineVersion the version of the engine used to to create the application.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html">Vulkan Docs/VkApplicationInfo</a>
 *
 * @ingroup group_Meta
 *
 */
void oriDefineStateApplicationInfo(oriState *state, const void *ext, unsigned int apiVersion, const char *name, unsigned int version, const char *engineName, unsigned int engineVersion) {
    if (!state) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return;
    }

    // using a compound literal should (?) be far less expensive than constantly dereferencing state to redefine the properties separately.
    state->appInfo = (VkApplicationInfo) {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        ext,
        name,
        version,
        engineName,
        engineVersion,
        apiVersion
    };

#   ifdef __oridebug
        _ori_DebugLog(
            "application info of state object at %p updated:\n"
            "\tAPI version: %d.%d.%d\n"
            "\tname : %s\n"
            "\tversion : %d.%d.%d\n"
            "\tengine name : %s\n"
            "\tengine version : %d.%d.%d\n"
            "\textensive structure : at %p",
            state,
            VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion),
            name,
            VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version),
            engineName,
            VK_VERSION_MAJOR(engineVersion), VK_VERSION_MINOR(engineVersion), VK_VERSION_PATCH(engineVersion),
            ext
        );
#   endif
}



// ============================================================================
// *****        LIBRARY MANAGEMENT                                        *****
// ============================================================================

/**
 * @brief Set a library-wide flag or value
 *
 * This function can be used to set a library-wide flag to configure your application.
 * The flags that can be set can be seen below. (in header)
 *
 * @param flag the flag to update
 * @param val the value to set the flag to
 *
 * @ingroup group_Meta
 *
 */
oriReturnStatus oriSetFlag(oriLibraryFlag flag, unsigned int val) {
#   ifdef __oridebug
        char flagstr[128];
#   endif

    switch (flag) {
        default:
#           ifdef __oridebug
                _ori_Warning("%s", "an invalid flag was given to oriSetFlag(); nothing was updated.");
#           endif

            return ORION_RETURN_STATUS_ERROR_INVALID_ENUM;
        case ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS:
#           ifdef __oridebug
                strncpy(flagstr, "ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS", 127);
#           endif

            _orion.flags.createInstanceDebugMessengers = val;
            break;
    }

#   ifdef __oridebug
        _ori_DebugLog("flag %s set to %d", flagstr, val);
#   endif

    return ORION_RETURN_STATUS_OK;
}

/**
 * @brief Optionally define the memory allocation functions to be used in Vulkan functions.
 *
 * This function sets the internally-held structure in which allocation function pointers can be defined.
 *
 * The structure passed to the @c callbacks parameter of this function will be referenced in any Vulkan function called internally by
 * the library with a @c pAllocator parameter.
 *
 * Passing @b NULL to this function will reset Vulkan to using default allocation callbacks as described by the implementation.
 *
 * @param callbacks the callbacks structure to use with Vulkan functions.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#memory-allocation">Vulkan Docs/Memory allocation</a>
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkAllocationCallbacks.html">Vulkan Docs/VkAllocationCallbacks</a>
 *
 * @ingroup group_Meta
 *
 */
void oriSetVulkanAllocationCallbacks(VkAllocationCallbacks callbacks) {
    _orion.callbacks.vulkanAllocators = &callbacks;
}
