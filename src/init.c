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
 * @brief Library and Vulkan initialisation, loading, and overall management
 *
 * @copyright Copyright (c) 2022
 *
 * This file defines functions involved with:
 *   - 'initialising' the library (i.e. creating a state object)
 *   - loading, initialising, and managing the Vulkan interface
 *   - the creation of some core library structures
 *   - the overall termination of the library
 *
 */

#include "orion.h"
#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// helper macro to remove duplicates from a dynamically allocated array
#define __RemoveArrayDuplicates(array, count) \
    { \
        for (unsigned int i = 0; i < count - 1; i++) { \
            if (array[i] != array[i + 1]) { \
                continue; \
            } \
            for (unsigned int j = i + 1; j < count - 1; j++) { \
                array[j] = array[j + 1]; \
            } \
            count--; \
            i--; \
        } \
    }



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        STATE                                                     *****
// ============================================================================

_ori_Lib _orion = {};


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
 * @sa oriFreeState()
 *
 * @ingroup group_Meta
 *
 */
oriState *oriCreateState() {
    oriState *r = malloc(sizeof(oriState));

    // init to ensure everything (most importantly linked list heads) start at NULL
    memset(r, 0, sizeof(oriState));

    _ori_DebugLog("state object created into %p", r);

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
void oriFreeState(oriState *state) {
    // first, we remove duplicates from the public arrays
    // this is because trying to destroy the state at the same address multiple times will lead to an exception.
    __RemoveArrayDuplicates(state->arrays.instances, state->arrays.instancesCount);

    // free VkInstances
    for (unsigned int i = 0; i < state->arrays.instancesCount; i++) {
        vkDestroyInstance(*state->arrays.instances[i], NULL);
        _ori_DebugLog("VkInstance at %p was freed by state object at location %p", state->arrays.instances[i], state);
        *state->arrays.instances[i] = NULL;
    }
    free(state->arrays.instances);
    state->arrays.instances = NULL;

    // finally, free state pointer
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
}

/**
 * @brief Define the properties of messages you want to be suppressed by the debug messenger of all instances to be created with a state object.
 *
 * This function only has an effect if the VK_EXT_debug_utils extension has been specified. This must be done before the respective call to
 * oriCreateStateInstance().
 *
 * If you call this function, the @ref section_main_Config "ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS" flag will be implicitly set to @b true.
 *
 * @note The message filters specified in this function are @b different to those specified in oriEnableDebugMessages().
 * Whilst those specified in that function are related to the Orion library, those specified here are related to the Vulkan API. Furthermore, properties
 * specified in oriEnableDebugMessages() describe the messages <b>to be displayed</b>, whilst those specified here describe the messages
 * <b>to be suppressed</b>.
 *
 * @param state the state to modify.
 * @param severities (bitmask) severities of the messages to @b suppress. By default, all messages are displayed <i>(if ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS is true)</i>
 * @param types (bitmask) types of the messages to @b suppress. By default, all messages are displayed <i>(if ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS is true)</i>
 *
 * @sa <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_EXT_debug_utils.html">Vulkan Docs/VK_EXT_debug_utils</a>
 * @sa
 * <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessageSeverityFlagBitsEXT.html">Vulkan Docs/VkDebugUtilsMessageSeverityFlagBitsEXT</a>
 * @sa <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessageTypeFlagBitsEXT.html">Vulkan Docs/VkDebugUtilsMessageTypeFlagBitsEXT</a>
 *
 * @ingroup group_VkAbstractions_Debugging
 *
 */
void oriDefineStateInstanceEnabledDebugMessages(oriState *state, VkDebugUtilsMessageSeverityFlagBitsEXT severities, VkDebugUtilsMessageTypeFlagBitsEXT types) {
    state->instanceCreateInfo.dbgmsngrEnabledMessages.severities = severities;
    state->instanceCreateInfo.dbgmsngrEnabledMessages.types = types;

    oriSetFlag(ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS, true);
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
    char flagstr[128];

    switch (flag) {
        default:
            _ori_Warning("%s", "An invalid flag was given to oriSetFlag(); nothing was updated.");
            return ORION_RETURN_STATUS_ERROR_INVALID_ENUM;
        case ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS:
            strncpy(flagstr, "ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS", 127);
            _orion.flags.createInstanceDebugMessengers = val;
            break;
    }

    _ori_DebugLog("flag %s set to %d", flagstr, val);
    return ORION_RETURN_STATUS_OK;
}
