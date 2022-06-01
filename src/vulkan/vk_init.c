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
 * @file vk_init.c
 * @author jack bennett
 * @brief Vulkan initialisation, loading, and overall management
 *
 * @copyright Copyright (c) 2022
 *
 * This file defines functions involved with loading, initialising, and managing
 * the Vulkan interface with the use of Orion abstractions.
 *
 */

#include "orion.h"
#include "orion_structs.h"
#include "orion_codes.h"
#include "orion_funcs.h"
#include "orion_helpers.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        CORE VULKAN API ABSTRACTIONS                              *****
// ============================================================================

/**
 * @brief Apply all previous properties set to the given @c state and create a VkInstance object, stored in @c state.
 *
 * Properties previously passed to @c state in functions such as oriDefineStateApplicationInfo(), and features specified in functions like oriFlagLayerEnabled(), will
 * be applied and used to create a <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html">VkInstance</a> object.
 *
 * The resulting instance object will be managed by the state.
 *
 * @param state the state object from which properties will be used, and to which the resulting VkInstance will be tied.
 * @return the resulting VkInstance opaque
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html">Vulkan Docs/VkInstance</a>
 *
 * @ingroup group_VkAbstractions_Core
 *
 */
oriReturnStatus oriCreateInstance(oriState *state, const void *ext, VkInstance *instancePtr) {
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = ext;

    createInfo.pApplicationInfo = &state->appInfo;

    // specify layers to be enabled
    createInfo.enabledLayerCount = state->instanceCreateInfo.enabledLayerCount;

    char logstr[768];
    memset(logstr, 0, sizeof(logstr));
    snprintf(logstr, 768, "VkInstance created into %p and will be managed by state object at location %p", instancePtr, state);

    // the support of these layers was already checked in oriFlagLayerEnabled().
    if (state->instanceCreateInfo.enabledLayerCount) {
        char logstr_layer[768];
        snprintf(logstr_layer, 768, "\n\tlayers enabled for this instance:\n");

        // add to log message
        for (unsigned int i = 0; i < state->instanceCreateInfo.enabledLayerCount; i++) {
            char s[768];
            snprintf(s, 768, "\t\t- name '%s'%s", state->instanceCreateInfo.enabledLayers[i], (i < state->instanceCreateInfo.enabledLayerCount - 1) ? "\n" : "");
            strncat(logstr_layer, s, 767); // GCC wants strncat to have one less than the length of dest, so instead of 768, we specify 767.
        }

        strncat(logstr, logstr_layer, 767);

        createInfo.ppEnabledLayerNames = (const char *const *) state->instanceCreateInfo.enabledLayers;
    } else {
        createInfo.ppEnabledLayerNames = NULL;
    }

    // now for extensions ...

    // check for VK_EXT_debug_utils, if it is enabled then make an instance debug messenger
    bool debugUtilsExtFound = false;

    if (state->instanceCreateInfo.enabledExtCount) {
        // unlike layers, the support of the specified extensions has not yet been checked
        // this is because the layers that provide some extensions might not yet have been specified
        // but we can now check for the support of each extension, removing the extension from the list if it is not supported.
        oriPruneInstanceExtensions(state);

        // check if the debug utils extension was specified
        for (unsigned int i = 0; i < state->instanceCreateInfo.enabledExtCount; i++) {
            // store current extension on stack to reduce dereferences
            if (!strcmp(state->instanceCreateInfo.enabledExtensions[i], "VK_EXT_debug_utils")) {
                debugUtilsExtFound = true;
                break;
            }
        }

        char logstr_ext[768];
        snprintf(logstr_ext, 768, "\n\tinstance extensions enabled for this instance:\n");

        // add to log message
        for (unsigned int i = 0; i < state->instanceCreateInfo.enabledExtCount; i++) {
            char s[768];
            snprintf(s, 768, "\t\t- name '%s'%s", state->instanceCreateInfo.enabledExtensions[i], (i < state->instanceCreateInfo.enabledExtCount - 1) ? "\n" : "");
            strncat(logstr_ext, s, 767); // GCC wants strncat to have one less than the length of dest, so instead of 768, we specify 767.
        }

        strncat(logstr, logstr_ext, 767);

        // any unsupported extensions have now been set to NULL in the array
        createInfo.ppEnabledExtensionNames = (const char *const *) state->instanceCreateInfo.enabledExtensions;
        createInfo.enabledExtensionCount = state->instanceCreateInfo.enabledExtCount;
    } else {
        createInfo.ppEnabledExtensionNames = NULL;
    }

    // give a warning if the instance debug messenger was desired but not available; none will be made
    if (_orion.flags.createInstanceDebugMessengers && !debugUtilsExtFound) {
        _ori_Warning("%s", "vulkan instance debug messenger requested but the required extension, VK_EXT_debug_utils, was not enabled. none will be created!");
    }

    // populate debug messenger info if necessary
    VkDebugUtilsMessengerCreateInfoEXT dbgmsngrCreateInfo = {};

    if (_orion.flags.createInstanceDebugMessengers && debugUtilsExtFound) {
        dbgmsngrCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        // severity and type is specified with oriSpecifyInstanceDebugMessages().
        dbgmsngrCreateInfo.messageSeverity = state->instanceCreateInfo.dbgmsngrEnabledMessages.severities;
        dbgmsngrCreateInfo.messageType = state->instanceCreateInfo.dbgmsngrEnabledMessages.types;

        // use internal callback, which routes Vulkan info to the Orion error callback.
        dbgmsngrCreateInfo.pfnUserCallback = _ori_VulkanDebugMessengerCallback;
        dbgmsngrCreateInfo.pUserData = _orion.callbacks.errorCallbackUserData;

        createInfo.pNext = &dbgmsngrCreateInfo;

        _ori_Notification("appended instance debug messenger for next instance (at %p)", instancePtr);
    }

    if (vkCreateInstance(&createInfo, NULL, instancePtr)) {
        _ori_ThrowError(ORERR_VULKAN_RETURN_ERROR);
        return ORION_RETURN_STATUS_ERROR_VULKAN_ERROR;
    }

    // add given instance pointer to state array
    _ori_AppendOntoDArray(VkInstance *, state->arrays.instances, state->arrays.instancesCount, instancePtr);

    _ori_Notification("%s", logstr);

    return ORION_RETURN_STATUS_OK;
}
