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
// *****        CORE VULKAN API ABSTRACTIONS: INSTANCES                   *****
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
oriReturnStatus oriCreateInstance(oriState *state, const void *ext, VkInstance *instanceOut) {
    // instanceOut can technically be NULL. in which case there is no point doing any of this.
    if (!instanceOut) {
        return ORION_RETURN_STATUS_OK;
    }

    // according to the Vulkan specification, pInstance (instanceOut) must be a valid pointer to a VkInstance handle
    if (!state) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return ORION_RETURN_STATUS_ERROR_NULL_POINTER;
    }

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = ext;

    createInfo.pApplicationInfo = &state->appInfo;

    // specify layers to be enabled
    createInfo.enabledLayerCount = state->instanceCreateInfo.enabledLayerCount;

#   ifdef __oridebug
        char logstr[768];
        memset(logstr, 0, sizeof(logstr));
        snprintf(logstr, 768, "VkInstance created into %p and will be managed by state object at location %p", instanceOut, state);
#   endif

    // the support of these layers was already checked in oriFlagLayerEnabled().
    if (state->instanceCreateInfo.enabledLayerCount) {
#       ifdef __oridebug
            char logstr_layer[768];
            snprintf(logstr_layer, 768, "\n\tlayers enabled for this instance:\n");

            // add to log message
            for (unsigned int i = 0; i < state->instanceCreateInfo.enabledLayerCount; i++) {
                char s[768];
                snprintf(s, 768, "\t\t- name '%s'%s", state->instanceCreateInfo.enabledLayers[i], (i < state->instanceCreateInfo.enabledLayerCount - 1) ? "\n" : "");
                strncat(logstr_layer, s, 767); // GCC wants strncat to have one less than the length of dest, so instead of 768, we specify 767.
            }

            strncat(logstr, logstr_layer, 767);
#       endif

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
        // (if requested)
        if (_orion.flags.createInstanceDebugMessengers) {
            for (unsigned int i = 0; i < state->instanceCreateInfo.enabledExtCount; i++) {
                if (!strcmp(state->instanceCreateInfo.enabledExtensions[i], "VK_EXT_debug_utils")) {
                    debugUtilsExtFound = true;
                    break;
                }
            }
        }

#       ifdef __oridebug
            char logstr_ext[768];
            snprintf(logstr_ext, 768, "\n\tinstance extensions enabled for this instance:\n");

            // add to log message
            for (unsigned int i = 0; i < state->instanceCreateInfo.enabledExtCount; i++) {
                char s[768];
                snprintf(s, 768, "\t\t- name '%s'%s", state->instanceCreateInfo.enabledExtensions[i], (i < state->instanceCreateInfo.enabledExtCount - 1) ? "\n" : "");
                strncat(logstr_ext, s, 767); // GCC wants strncat to have one less than the length of dest, so instead of 768, we specify 767.
            }

            strncat(logstr, logstr_ext, 767);
#       endif

        // any unsupported extensions have now been set to NULL in the array
        createInfo.ppEnabledExtensionNames = (const char *const *) state->instanceCreateInfo.enabledExtensions;
        createInfo.enabledExtensionCount = state->instanceCreateInfo.enabledExtCount;
    } else {
        createInfo.ppEnabledExtensionNames = NULL;
    }

    // give a warning if the instance debug messenger was desired but not available; none will be made
#   ifdef __oridebug
        if (_orion.flags.createInstanceDebugMessengers && !debugUtilsExtFound) {
            _ori_Warning("%s", "vulkan instance debug messenger requested but the required extension, VK_EXT_debug_utils, was not enabled. none will be created!");
        }
#   endif

    // populate debug messenger info if necessary
    VkDebugUtilsMessengerCreateInfoEXT dbgmsngrCreateInfo = {};

    // debugUtilsExtFound can only be true if _orion.flags.createInstanceDebugMessengers is also true
    if (debugUtilsExtFound) {
        dbgmsngrCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        // severity and type is specified with oriSpecifyInstanceDebugMessages().
        dbgmsngrCreateInfo.messageSeverity = state->instanceCreateInfo.dbgmsngrEnabledMessages.severities;
        dbgmsngrCreateInfo.messageType = state->instanceCreateInfo.dbgmsngrEnabledMessages.types;

        // use internal callback, which routes Vulkan info to the Orion error callback.
        dbgmsngrCreateInfo.pfnUserCallback = _ori_VulkanDebugMessengerCallback;
        dbgmsngrCreateInfo.pUserData = _orion.callbacks.errorCallbackUserData;

        createInfo.pNext = &dbgmsngrCreateInfo;

#       ifdef __oridebug
            _ori_Notification("appended instance debug messenger for upcoming instance creation (at %p)", instanceOut);
#       endif
    }

    if (vkCreateInstance(&createInfo, _orion.callbacks.vulkanAllocators, instanceOut)) {
        _ori_ThrowError(ORERR_VULKAN_RETURN_ERROR);
        return ORION_RETURN_STATUS_ERROR_VULKAN_ERROR;
    }

    // add given instance pointer to state array
    _ori_AppendOntoDArray(VkInstance *, state->arrays.instances, state->arrays.instancesCount, instanceOut);

#   ifdef __oridebug
        _ori_Notification("%s", logstr);
#   endif

    return ORION_RETURN_STATUS_OK;
}



// ============================================================================
// *****        CORE VULKAN API ABSTRACTIONS: DEVICES                     *****
// ============================================================================

/**
 * @brief Get a list of all devices that support Vulkan and are considered suitable for the application.
 *
 * This function retrieves an array of physical devices accessible to a Vulkan instance that are considered suitable
 * for the application.
 *
 * @param instance Vulkan instance to query
 * @param checkFunc an @ref oriPhysicalDeviceSuitabilityCheckFunc function pointer that returns true if a device is suitable and false
 * if not.
 * @param countOut pointer to a variable into which the number of suitable physical devices will be returned.
 * @param devicesOut pointer to an array into which the list of suitable physical devices will be returned.
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @ingroup group_VkAbstractions_Core_Devices
 *
 */
oriReturnStatus oriEnumerateSuitablePhysicalDevices(VkInstance instance, oriPhysicalDeviceSuitabilityCheckFunc checkFunc, unsigned int *countOut, VkPhysicalDevice **devicesOut) {
    if (!devicesOut && !countOut) {
#       ifdef __oridebug
            _ori_Warning("%s", "oriEnumerateSuitablePhysicalDevices() was called but all output pointers were passed as NULL");
#       endif

        return ORION_RETURN_STATUS_OK;
    }

    // c = number of available devices
    // cr = number of suitable devices (will be returned into countOut)
    unsigned int c = 0, cr = 0;

    // get number of available devices
    if (vkEnumeratePhysicalDevices(instance, &c, NULL)) {
        _ori_ThrowError(ORERR_VULKAN_RETURN_ERROR);
        return ORION_RETURN_STATUS_ERROR_VULKAN_ERROR;
    }

    // no vulkan-supported GPUs were found
    if (!c) {
#       ifdef __oridebug
            _ori_Warning("%s", "couldn't find physical device with Vulkan support");
#       endif

        return ORION_RETURN_STATUS_OK;
    }

    // d = array of available devices
    VkPhysicalDevice *d = calloc(c, sizeof(VkPhysicalDevice));
    if (!d) {
        printf("Memory error -- calloc returned null!\n");
        return ORION_RETURN_STATUS_MEMORY_ERROR;
    }

    // get array of available devices
    if (vkEnumeratePhysicalDevices(instance, &c, d)) {
        _ori_ThrowError(ORERR_VULKAN_RETURN_ERROR);
        return ORION_RETURN_STATUS_ERROR_VULKAN_ERROR;
    }

    if (devicesOut) {
        // initialise devices to NULL
        *devicesOut = NULL;
    }

    for (unsigned int i = 0; i < c; i++) {
        // check if each device is suitable, if checkFunc is not NULL
        if (checkFunc && !checkFunc(d[i])) {
            continue;
        }

        if (devicesOut) {
            _ori_AppendOntoDArray(VkPhysicalDevice, *devicesOut, cr, d[i]);
        } else { // if devicesOut is NULL, then countOut must not be (both cannot be NULL, as asserted at the beginning of the function)
            cr++;
        }
    }

    if (countOut) {
        *countOut = cr;
    }

    // now cr has been updated, we check if it is 0 (i.e. no devices were deemed suitable)
    if (!cr) {
        // make sure we free the array of available devices despite returning early
        // the parameter array would not have been allocated if this is reached so we don't need to worry about that
        free(d);
        d = NULL;

#       ifdef __oridebug
            _ori_Warning("found %d available physical device%s, but none were determined suitable", c, (c != 1) ? "s" : "");
#       endif

        return ORION_RETURN_STATUS_OK;
    }

#   ifdef __oridebug
        if (checkFunc) {
            _ori_DebugLog("found %d available physical device%s, of which %d %s determined suitable", c, (c != 1) ? "s" : "", cr, (cr != 1) ? "were" : "was");
        } else {
            _ori_DebugLog("found %d available physical device%s, no suitability check function used", c, (c != 1) ? "s" : "");
        }
#   endif

    free(d);
    d = NULL;

    return ORION_RETURN_STATUS_OK;
}

/**
 * @brief Get a list of all available queue families for a specified physical device.
 *
 * This function retrieves an array of queue families available to a specified physical device.
 *
 * @param physicalDevice the physical device to query.
 * @param familiesOut pointer to a variable into which the array of queue families will be returned.
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @ingroup group_VkAbstractions_Core_Devices
 *
 */
oriReturnStatus oriEnumerateAvailableQueueFamilies(VkPhysicalDevice *physicalDevice, unsigned int *countOut, VkQueueFamilyProperties **familiesOut) {
    if (!countOut && !familiesOut) {
#       ifdef __oridebug
            _ori_Warning("%s", "oriEnumerateAvailableQueueFamilies() was called but all output pointers were passed as NULL");
#       endif

        return ORION_RETURN_STATUS_OK;
    }

    if (!physicalDevice) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return ORION_RETURN_STATUS_ERROR_NULL_POINTER;
    }

    unsigned int c = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &c, NULL);

    if (countOut) {
        *countOut = c;
    }

    if (!c) {
#       ifdef __oridebug
            _ori_Warning("%s", "no queue families available for a specified physical device");
#       endif

        return ORION_RETURN_STATUS_OK;
    }

    if (!familiesOut) {
        return ORION_RETURN_STATUS_OK;
    }

    *familiesOut = calloc(c, sizeof(VkQueueFamilyProperties));
    if (!*familiesOut) {
        printf("Memory error -- calloc returned null!\n");
        return ORION_RETURN_STATUS_MEMORY_ERROR;
    }

    vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &c, *familiesOut);

#   ifdef __oridebug
        _ori_DebugLog("found %d available queue famil%s available to physical device at %p", c, (c != 1) ? "ies" : "y", physicalDevice);
#   endif

    return ORION_RETURN_STATUS_OK;
}

/**
 * @brief Create a logical device to connect to one or more physical devices.
 *
 * The resulting logical device will be managed by @c state.
 *
 * If @c physicalDeviceCount is 1, then @c physicalDevices is a pointer to the desired physical device.
 * If @c physicalDeviceCount is more than 1, then @c physicalDevices is an array of desired physical devices in the same device group.
 * @c physicalDeviceCount @b cannot be 0.
 *
 * See <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html">Vulkan Docs/VkDeviceCreateInfo</a> and, occasionally,
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceGroupDeviceCreateInfo.html">Vulkan Docs/VkDeviceGroupDeviceCreateInfo</a>
 * for more info.
 *
 * @param state the state which will manage the resulting logical device.
 * @param physicalDeviceCount the amount of physical devices to create the logical device for.
 * @param physicalDevices either a pointer to the physical device or an array of physical devices (see above).
 * @param ext equivalent to the @c pNext parameter in the Vulkan Specification (linked below): NULL or a pointer to a structure extending this structure.
 * @param queueCreateInfoCount the size of the @c queueCreateInfos array.
 * @param queueCreateInfos array of
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceQueueCreateInfo.html">VkDeviceQueueCreateInfo</a> structures describing
 * the queues that are requested along with the logical device.
 * @param extensionCount the size of the @c extensionNames array.
 * @param extensionNames array of null-terminated UTF-8 strings containing the names of device extensions to be enabled for the logical device.
 * @param enabledFeatures NULL or a pointer to a
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html">VkPhysicalDeviceFeatures</a> structure containing
 * flags of device features to be enabled.
 * @param deviceOut pointer to the variable into which the resulting device will be returned.
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html">Vulkan Docs/VkDeviceCreateInfo</a>
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceGroupDeviceCreateInfo.html">Vulkan Docs/VkDeviceGroupDeviceCreateInfo</a>
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCreateDevice.html">Vulkan Docs/vkCreateDevice()</a>
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceQueueCreateInfo.html">Vulkan Docs/VkDeviceQueueCreateInfo</a>
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html">Vulkan Docs/VkPhysicalDeviceFeatures</a>
 *
 * @ingroup group_VkAbstractions_Core_Devices
 *
 */
oriReturnStatus oriCreateLogicalDevice(oriState *state, unsigned int physicalDeviceCount, VkPhysicalDevice *physicalDevices, const void *ext, unsigned int queueCreateInfoCount, VkDeviceQueueCreateInfo *queueCreateInfos, unsigned int extensionCount, const char **extensionNames, VkPhysicalDeviceFeatures *enabledFeatures, VkDevice *deviceOut) {
    if (!deviceOut) {
#       ifdef __oridebug
            _ori_Warning("%s", "oriCreateLogicalDevice() was called but all output pointers were passed as NULL");
#       endif

        return ORION_RETURN_STATUS_OK;
    }

    if (!state || !physicalDeviceCount || !physicalDevices) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return ORION_RETURN_STATUS_ERROR_NULL_POINTER;
    }

#   ifdef __oridebug
        char logstr[768];
        memset(logstr, 0, sizeof(logstr));
        snprintf(logstr, 768, "VkDevice created into %p and will be managed by state object at location %p", deviceOut, state);
#   endif

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = ext;

    // append queue info to log string
#   ifdef __oridebug
        {
            char s[768];
            snprintf(s, 768, "\n\t%d queue%s %s requested", queueCreateInfoCount, (queueCreateInfoCount == 1) ? "" : "s", (queueCreateInfoCount == 1) ? "was" : "were");
            strncat(logstr, s, 767);
        }
#   endif

    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;

    // append device extension info to log string
#   ifdef __oridebug
        {
            char s[768];
            snprintf(s, 768, "\n\t%d extension%s %s enabled for this device", extensionCount, (extensionCount == 1) ? "" : "s", (extensionCount == 1) ? "was" : "were");

            // if there were any extensions enabled, list them
            if (extensionCount) {
                strncat(s, ":\n", 767);

                // add to log message
                for (unsigned int i = 0; i < extensionCount; i++) {
                    char _s[768];
                    snprintf(_s, 768, "\t\t- name '%s'%s", extensionNames[i], (i < extensionCount - 1) ? "\n" : "");
                    strncat(s, _s, 767);
                }
            }

            strncat(logstr, s, 767);
        }
#   endif

    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensionNames;

    createInfo.pEnabledFeatures = enabledFeatures;

    // this won't be updated if physicalDeviceCount is 1
    VkDeviceGroupDeviceCreateInfo groupCreateInfo = {};

    // append a device group create info to the device create info if there are multiple physical devices
    if (physicalDeviceCount > 1) {
        groupCreateInfo.physicalDeviceCount = physicalDeviceCount;
        groupCreateInfo.pPhysicalDevices = physicalDevices;

        createInfo.pNext = &groupCreateInfo;
    }

    // we pass the first element of physicalDevices to vkCreateDevice - even if multiple are specified:
    // "if physicalDeviceCount is not 0, the physicalDevice parameter of vkCreateDevice must be an element of pPhysicalDevices" - from VkDeviceGroupDeviceCreateInfo(3)
    if (vkCreateDevice(*physicalDevices, &createInfo, NULL, deviceOut)) {
        _ori_ThrowError(ORERR_VULKAN_RETURN_ERROR);
        return ORION_RETURN_STATUS_ERROR_VULKAN_ERROR;
    }

    _ori_AppendOntoDArray(VkDevice *, state->arrays.logicalDevices, state->arrays.logicalDevicesCount, deviceOut);

#   ifdef __oridebug
        _ori_Notification("%s", logstr);
#   endif

    return ORION_RETURN_STATUS_OK;
}
