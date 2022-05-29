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
 * the Vulkan interface with the use of Orion abstractions
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
oriReturnStatus oriCreateStateInstance(oriState *state, VkInstance *instancePtr) {
    oriReturnStatus r = ORION_RETURN_STATUS_OK;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

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

    // now for extensions
    createInfo.enabledExtensionCount = state->instanceCreateInfo.enabledExtCount;

    // check for VK_EXT_debug_utils, if it is enabled then make an instance debug messenger
    bool debugUtilsExtFound = false;

    // unlike layers, the support of the specified extensions has not yet been checked
    // this is because the layers that provide some extensions might not yet have been specified
    // but we can now check for the support of each extension, removing the extension from the list if it is not supported.
    // obviously, there will be a big warning in the debug output if any desired extensions are not loaded
    if (state->instanceCreateInfo.enabledExtCount) {
        for (unsigned int i = 0; i < state->instanceCreateInfo.enabledExtCount; i++) {
            // store current extension on stack to reduce dereferences
            const char *curext = state->instanceCreateInfo.enabledExtensions[i];

            // check if the extension is already provided by the implementation
            if (oriCheckInstanceExtensionAvailability(curext, NULL)) {
                if (!strcmp(curext, "VK_EXT_debug_utils")) {
                    debugUtilsExtFound = true;
                }

                continue;
            }

            // otherwise:
            bool _providedByALayer = false;

            // if there are any layers, traverse through the array of them and check if they provide the given extension
            // set _providedByALayer to true on the first one that does
            for (unsigned int j = 0; j < state->instanceCreateInfo.enabledLayerCount; j++) {
                if (oriCheckInstanceExtensionAvailability(curext, state->instanceCreateInfo.enabledLayers[j])) {
                    _providedByALayer = true;
                    break;
                }
            }

            if (!_providedByALayer) {
                // if this is reached, then we know the extension is not provided by the implementation and there are no layers that could provide it
                // so we know the extension is not available
                _ori_Warning("specified instance extension `%s` was not found", curext);
                r = ORION_RETURN_STATUS_ERROR_NOT_FOUND;

                // set array element to NULL
                free(state->instanceCreateInfo.enabledExtensions[i]);
                state->instanceCreateInfo.enabledExtensions[i] = NULL;

                // decrease counter by one
                // (the state variable doesn't matter anymore so we just change the create info count)
                createInfo.enabledExtensionCount--;
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

        // severity and type is specified with oriDefineStateInstanceEnabledDebugMessages().
        dbgmsngrCreateInfo.messageSeverity = state->instanceCreateInfo.dbgmsngrEnabledMessages.severities;
        dbgmsngrCreateInfo.messageType = state->instanceCreateInfo.dbgmsngrEnabledMessages.types;

        // use internal callback, which routes Vulkan info to the Orion error callback.
        dbgmsngrCreateInfo.pfnUserCallback = _ori_VulkanDebugMessengerCallback;
        dbgmsngrCreateInfo.pUserData = _orion.callbacks.errorCallbackUserData;

        createInfo.pNext = &dbgmsngrCreateInfo;
    }

    if (vkCreateInstance(&createInfo, NULL, instancePtr)) {
        r = ORION_RETURN_STATUS_ERROR_VULKAN_ERROR;
        _ori_ThrowError(ORERR_VULKAN_RETURN_ERROR);
    }

    // add given pointer to instance to state array
    state->arrays.instancesCount++;
    state->arrays.instances = realloc(state->arrays.instances, state->arrays.instancesCount * sizeof(VkInstance *));
    if (!state->arrays.instances) {
        r = ORION_RETURN_STATUS_MEMORY_ERROR;
        _ori_ThrowError(ORERR_MEMORY_ERROR);
    }
    state->arrays.instances[state->arrays.instancesCount - 1] = instancePtr;

    // we no longer need the data used for the create info struct so we can free it all
    {
        for (unsigned int i = 0; i < state->instanceCreateInfo.enabledLayerCount; i++) {
            free(state->instanceCreateInfo.enabledLayers[i]);
            state->instanceCreateInfo.enabledLayers[i] = NULL;
        }
        state->instanceCreateInfo.enabledLayerCount = 0;

        free(state->instanceCreateInfo.enabledLayers);
        state->instanceCreateInfo.enabledLayers = NULL;
    }
    {
        for (unsigned int i = 0; i < state->instanceCreateInfo.enabledExtCount; i++) {
            free(state->instanceCreateInfo.enabledExtensions[i]);
            state->instanceCreateInfo.enabledExtensions[i] = NULL;
        }
        state->instanceCreateInfo.enabledExtCount = 0;

        free(state->instanceCreateInfo.enabledExtensions);
        state->instanceCreateInfo.enabledExtensions = NULL;
    }

    _ori_Notification(logstr, 0);

    return r;
}
