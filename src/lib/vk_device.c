/* *************************************************************************************** */
/*                       ORION GRAPHICS LIBRARY AND RENDERING ENGINE                       */
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


// ============================================================================ //
// *****                     Doxygen file information                     ***** //
// ============================================================================ //

/**
 * @file vk_device.c
 * @author jack bennett
 * @brief Vulkan device management
 *
 * @copyright Copyright (c) 2022 jack bennett
 *
 * This file contains functions broadly related to the management of graphics devices with Vulkan,
 * such as the enumeration of available and suitable physical devices and choosing the best
 * one for the application.
 *
 */

#include "orion.h"
#include "orion_errors.h"
#include "orion_flags.h"
#include "orion_funcs.h"
#include "orion_structs.h"

#include <stdio.h>


// ----[Orion library public interface]---------------------------------------- //
//                          Vulkan device management                            //

const oriReturnStatus_t oriCreateLogicalDevice(
    VkDevice *deviceOut,
    const VkDeviceCreateFlags deviceFlags,
    const VkPhysicalDevice physicalDevice,
    const unsigned int queueCreateInfoCount,
    const VkDeviceQueueCreateInfo *queueCreateInfos,
    const unsigned int extensionCount,
    const char **extensionNames,
    VkPhysicalDeviceFeatures *enabledFeatures,
    const void *deviceNext
) {
    if (!deviceOut) { // no outputs given
        _oriWarning("all output variables NULL in call to %s", __func__);
        return ORION_RETURN_STATUS_NO_OUTPUT;
    }
    if (!physicalDevice) { // there has to be a physical device referenced
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }
    if (!queueCreateInfos && queueCreateInfoCount) { // no queues specified but count is not 0
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }
    if (!extensionNames && extensionCount) { // no exts given but count is not 0
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }

    // check if the given VkDevice pointer is already in the hash table of logical devices
    {
        _oriVkDevice_t *queryStructure;
        HASH_FIND_PTR(_orion.allocatees.vkDevices, deviceOut, queryStructure);
        if (queryStructure) { // uthash will have set queryStructure to NULL if it wasn't found so we can rely on this check
            _oriWarning("orion already allocated logical device at %p (%s)", deviceOut, __func__);
            return ORION_RETURN_STATUS_SKIPPED;
        }
    }

    // init device create-info struct
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = deviceNext;
    createInfo.flags = deviceFlags;
    createInfo.pEnabledFeatures = enabledFeatures;

    // these are deprecated and ignored: https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;

    // there isn't really as much of a way to validate the specified queues as there is for extensions.
    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;

#   ifdef __oridebug
        // debug message will be logged at the end of the function if successful
        char logstr[MAX_LOG_LEN];
        memset(logstr, 0, MAX_LOG_LEN);

        // we can also append the amount of queues as this won't change throughout the function
        snprintf(logstr, MAX_LOG_LEN, "logical device created into %p (%s)\n\t%u queues requested", deviceOut, __func__, queueCreateInfoCount);
#   endif

    // static array that will hold the compatible extensions
    // we are storing this in the primary function scope because they are referenced by vkCreateInstance() and so must be preserved until then.
    const char *actualEnabledExts[extensionCount];
    unsigned int actualEnabledExtCount = 0;

    // validate extensions if there were any specified
    if (extensionCount && extensionNames) {
#       ifdef __oridebug
            // string will be concatenated onto logstr
            char logstr_extlist[MAX_LOG_LEN];
            memset(logstr_extlist, 0, MAX_LOG_LEN);
#       endif

        // iterate through extensions
        for (unsigned int i = 0; i < extensionCount; i++) {
            // we use this design instead of continuing so the debug message is ensured to be concatenated out if __oridebug is enabled*
            bool provided = false;

            // check if the current extension is provided by the Vulkan implementation
            if (oriCheckDeviceExtensionAvailability(physicalDevice, extensionNames[i], NULL)) {
                actualEnabledExts[i] = extensionNames[i];
                actualEnabledExtCount++;
                provided = true;
            }

            if (!provided) {
                // iterate through each instance (there is rarely more than one so this is fine)
                _oriVkInstance_t *cur;
                for (cur = _orion.allocatees.vkInstances; cur != NULL; cur = cur->hh.next) {
                    // get layers
                    char **layers;
                    unsigned int layerCount;
                    if (oriEnumerateEnabledLayers(cur->handle, &layerCount, &layers)) {
                        return ORION_RETURN_STATUS_ERROR;
                    }

                    // iterate through each of the instance's layers
                    for (unsigned int j = 0; j < layerCount; j++) {
                        if (oriCheckDeviceExtensionAvailability(physicalDevice, extensionNames[j], layers[j])) {
                            actualEnabledExts[j] = extensionNames[j];
                            actualEnabledExtCount++;
                            provided = true;

                            // set cur to NULL to break out of the nesting loop
                            cur = NULL;

                            break;
                        }
                    }
                }
            }

            // *this is where the extension name is concatenated (if the current extension turned out to be available)
#           ifdef __oridebug
                if (provided) {
                    char s[MAX_LOG_LEN];
                    snprintf(s, MAX_LOG_LEN, "\n\t\t[%u] name '%s'", i, extensionNames[i]);
                    strncat(logstr_extlist, s, MAX_LOG_LEN);
                } else {
                    // print warning stating that the extension was not available
                    _oriWarning("device extension %s not provided by Vulkan implementation or any layers", extensionNames[i]);
                }
#           endif
        }

        // pass now-filtered extension array to Vulkan
        if (actualEnabledExtCount) {
            createInfo.enabledExtensionCount = actualEnabledExtCount;
            createInfo.ppEnabledExtensionNames = (const char *const *) actualEnabledExts;

#           ifdef __oridebug
                // concatenate onto logstr
                char s[MAX_LOG_LEN];
                snprintf(s, MAX_LOG_LEN, "\n\t%u extensions enabled:", actualEnabledExtCount);
                strncat(logstr, s, MAX_LOG_LEN);

                strncat(logstr, logstr_extlist, MAX_LOG_LEN);
#           endif
        }
    }

    if (vkCreateDevice(physicalDevice, &createInfo, _orion.callbacks.vulkanAllocators, deviceOut)) {
        _oriError(ORIERR_DEVICE_CREATION_FAIL, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    return ORION_RETURN_STATUS_OK;
}

const oriReturnStatus_t oriEnumerateSuitablePhysicalDevices(
    const VkInstance *instance,
    const oriPhysicalDeviceSuitabilityCheckfun checkFun,
    unsigned int *countOut,
    VkPhysicalDevice **devicesOut
) {
    if (!instance) { // instance is NULL
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }
    if (!countOut && !devicesOut) { // both countOut and devicesOut are NULL
        _oriWarning("all output variables NULL in call to %s", __func__);
        return ORION_RETURN_STATUS_NO_OUTPUT;
    }

    // c = number of available devices
    unsigned int c;

    // get number of available devices
    if (vkEnumeratePhysicalDevices(*instance, &c, NULL)) {
        _oriError(ORIERR_VULKAN_QUERY_FAIL, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    if (!c) {
        // no vulkan-supported GPUs were found
#       ifdef __oridebug
            _oriWarning("couldn't find any physical devices with Vulkan support (%s)", __func__);
#       endif

        return ORION_RETURN_STATUS_OK;
    }

    // d = array of available devices
    VkPhysicalDevice *d = malloc(c * sizeof(VkPhysicalDevice));
    if (!d) {
        _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    // get array of available devices
    if (vkEnumeratePhysicalDevices(*instance, &c, d)) {
        _oriError(ORIERR_VULKAN_QUERY_FAIL, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    // cr = number of suitable devices (will be returned into countOut)
    unsigned int cr;

    if (checkFun) {
        // iterate through each device and check its suitability
        for (unsigned int i = 0; i < c; i++) {
            if (checkFun(d[i])) {
                cr++;
            }
        }
    } else {
        // if checkFun is NULL then all available devices are considered suitable for the sake of this function
        cr = c;
    }

    // return cr into countOut as it won't change after this point
    if (countOut) {
        *countOut = cr;
    }

    // now that we have returned the count, the only thing left to do is return the devices.
    // however, deviceOut could be NULL, in which case no more work is needed.
    if (!devicesOut) {
        free(d); // (don't forget to free d)
        d = NULL;
        return ORION_RETURN_STATUS_OK;
    }

    // if there are no suitable or available devices
    if (!cr) {
        if (devicesOut) {
            *devicesOut = NULL;
        }

        free(d);
        d = NULL;
        return ORION_RETURN_STATUS_OK;
    };

    // if there are any suitable or available devices, then we can allocate and set devicesOut.
    // we do this through a buffer variable (dr) to reduce dereferences.
    VkPhysicalDevice *dr = malloc(cr * sizeof(VkPhysicalDevice));
    if (!dr) {
        _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    // iterate through the devices again, this time if they are suitable then we give them to dr.
    for (unsigned int i = 0; i < c; i++) {
        if (!checkFun || checkFun(d[i])) {
            dr[i] = d[i];
        }
    }

    // finally, return dr into devicesOut
    // it is now up to the user to free this space.
    *devicesOut = dr;

    free(d);
    d = NULL;
    return ORION_RETURN_STATUS_OK;
}

const oriReturnStatus_t oriEnumerateAvailableQueueFamilies(
    const VkPhysicalDevice *physicalDevice,
    unsigned int *countOut,
    VkQueueFamilyProperties **familiesOut
) {
    if (!physicalDevice) { // physicalDevice is NULL
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }
    if (!countOut && !familiesOut) { // both countOut and familiesOut are NULL
        _oriWarning("all output variables NULL in call to %s", __func__);
        return ORION_RETURN_STATUS_NO_OUTPUT;
    }

    // c = number of available families
    unsigned int c;

    // get number of available families
    // (this vk function returns void)
    vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &c, NULL);

    // return countOut
    if (countOut) {
        *countOut = c;
    }

    // if !familiesOut then no more work is needed
    if (!familiesOut) {
        return ORION_RETURN_STATUS_OK;
    }

    // if there were no families found then set familiesOut to NULL and return
    if (!c) {
        // existence of familiesOut was validated previously
        *familiesOut = NULL;

        return ORION_RETURN_STATUS_OK;
    }

    // allocate into familiesOut
    *familiesOut = malloc(c * sizeof(VkQueueFamilyProperties));
    if (!*familiesOut) {
        _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &c, *familiesOut);

    // all info has been returned
    return ORION_RETURN_STATUS_OK;
}
