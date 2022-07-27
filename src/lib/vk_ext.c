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
 * @file vk_ext.c
 * @author jack bennett
 * @brief Vulkan extensions and feature loading
 *
 * @copyright Copyright (c) 2022 jack bennett
 *
 * This file contains functions broadly related to the use of non-core Vulkan API features.
 *
 * This isn't specifically extensions, but rather instance + device extensions as well as layers.
 * ... and anything else Vulkan-related that isn't in the Core API.
 *
 */

#include "orion.h"
#include "orion_errors.h"
#include "orion_funcs.h"

#include <stdlib.h>
#include <string.h>


// ----[Orion library public interface]---------------------------------------- //
//                    Vulkan extensions and feature loading                     //

const bool oriCheckLayerAvailability(const char *layer) {
    if (!layer) { // layer is NULL
        _oriError(ORIERR_NULL_POINTER, __func__);
        return false;
    }

    unsigned int layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *availableLayers = malloc(layerCount * sizeof(VkLayerProperties));
    if (!availableLayers) {
        _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
        return false; // this won't be reached since _oriFatalError halts the program but might as well put it here
    }
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    // we will return at the end to ensure that availableLayers is freed
    bool found = false;

    for (unsigned int i = 0; i < layerCount; i++) {
        // check if any of the available layers is the specified layer
        if (!strcmp(availableLayers[i].layerName, layer)) {
            found = true;
            break;
        }
    }

#   ifdef __oridebug
        if (!found) {
            _oriLog("validation of layer '%s' failed (%s)", layer, __func__);
        }
#   endif

    free(availableLayers);
    availableLayers = NULL;

    return found;
}

const bool oriCheckLayerEnabled(const VkInstance *instance, const char *layer) {
    if (!instance || !layer) { // necessary parameter is NULL
        _oriError(ORIERR_NULL_POINTER, __func__);
        return false;
    }

    // get array of enabled layers
    unsigned int count;
    char **arr;
    if (oriEnumerateEnabledLayers(instance, &count, &arr)) {
        // oriEnumerateEnabledLayers would have sent output to stdout so we just need to return here
        return false;
    }

    for (unsigned int i = 0; i < count; i++) {
        // if there is a match
        if (!strcmp(arr[i], layer)) {
            return true;
        }
    }

    // there were no matches
    return false;
}

const oriReturnStatus_t oriEnumerateEnabledLayers(const VkInstance *instance, unsigned int *layerCountOut, char ***layerNamesOut) {
    if (!instance) { // instance is NULL
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }
    if (!layerCountOut && !layerNamesOut) { // both layerCount and layerNames are NULL
        _oriWarning("all output variables NULL in call to %s", __func__);
        return ORION_RETURN_STATUS_NO_OUTPUT;
    }

    _oriVkInstance_t *instanceWrapper;
    HASH_FIND_PTR(_orion.allocatees.vkInstances, &instance, instanceWrapper);

    // if the instance was not found in the hash table, then it was not created with Orion (or it was not created at all)
    if (!instanceWrapper) {
        _oriError(ORIERR_INVALID_OBJECT, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    if (layerCountOut) {
        *layerCountOut = instanceWrapper->layerCount;
    }
    if (layerNamesOut) {
        *layerNamesOut = instanceWrapper->layers;
    }

    return ORION_RETURN_STATUS_OK;
}

const bool oriCheckInstanceExtensionAvailability(const char *extension, const char *layer) {
    if (!extension) { // extension is NULL
        _oriError(ORIERR_NULL_POINTER, __func__);
        return false;
    }

    unsigned int extCount = 0;
    vkEnumerateInstanceExtensionProperties(layer, &extCount, NULL);

    VkExtensionProperties *availableExts = malloc(extCount * sizeof(VkExtensionProperties));
    if (!availableExts) {
        _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
        return false; // this won't be reached since _oriFatalError halts the program but might as well put it here
    }
    vkEnumerateInstanceExtensionProperties(layer, &extCount, availableExts);

    // we will return at the end to ensure that availableExts is freed
    bool found = false;

    for (unsigned int i = 0; i < extCount; i++) {
        // check if any of the available extensions is the specified extension
        if (!strcmp(availableExts[i].extensionName, extension)) {
            found = true;
            break;
        }
    }

#   ifdef __oridebug
        if (!found) {
            _oriLog("validation of instance extension '%s' failed (not provided by %s) (%s)", extension, (layer) ? layer : "implementation", __func__);
        }
#   endif

    free(availableExts);
    availableExts = NULL;

    return found;
}

const bool oriCheckInstanceExtensionEnabled(const VkInstance *instance, const char *extension) {
    if (!instance || !extension) { // necessary parameter is NULL
        _oriError(ORIERR_NULL_POINTER, __func__);
        return false;
    }

    // get array of enabled extensions
    unsigned int count;
    char **arr;
    if (oriEnumerateEnabledInstanceExtensions(instance, &count, &arr)) {
        // oriEnumerateEnabledInstanceExtensions would have sent output to stdout so we just need to return here
        return false;
    }

    for (unsigned int i = 0; i < count; i++) {
        // if there is a match
        if (!strcmp(arr[i], extension)) {
            return true;
        }
    }

    // there were no matches
    return false;
}

const oriReturnStatus_t oriEnumerateEnabledInstanceExtensions(const VkInstance *instance, unsigned int *extCountOut, char ***extNamesOut) {
    if (!instance) { // instance is NULL
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }
    if (!extCountOut && !extNamesOut) { // both extCountOut and extNamesOut are NULL
        _oriWarning("all output variables NULL in call to %s", __func__);
        return ORION_RETURN_STATUS_NO_OUTPUT;
    }

    _oriVkInstance_t *instanceWrapper;
    HASH_FIND_PTR(_orion.allocatees.vkInstances, &instance, instanceWrapper);

    // if the instance was not found in the hash table, then it was not created with Orion (or it was not created at all)
    if (!instanceWrapper) {
        _oriError(ORIERR_INVALID_OBJECT, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    if (extCountOut) {
        *extCountOut = instanceWrapper->extensionCount;
    }
    if (extNamesOut) {
        *extNamesOut = instanceWrapper->extensions;
    }

    return ORION_RETURN_STATUS_OK;
}

const bool oriCheckDeviceExtensionAvailability(const VkPhysicalDevice physicalDevice, const char *extension, const char *layer) {
    if (!physicalDevice || !extension) { // a required parameter is NULL
        _oriError(ORIERR_NULL_POINTER, __func__);
        return false;
    }

    unsigned int extCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, layer, &extCount, NULL);

    VkExtensionProperties *availableExts = malloc(extCount * sizeof(VkExtensionProperties));
    if (!availableExts) {
        _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
        return false;
    }
    vkEnumerateDeviceExtensionProperties(physicalDevice, layer, &extCount, availableExts);

    // we will return at the end to ensure that availableExts is freed
    bool found = false;

    for (unsigned int i = 0; i < extCount; i++) {
        // check if any of the available extensions is the specified extension
        if (!strcmp(availableExts[i].extensionName, extension)) {
            found = true;
            break;
        }
    }

#   ifdef __oridebug
        if (!found) {
            _oriLog("validation of device extension '%s' failed (%s)", extension, __func__);
        }
#   endif

    free(availableExts);
    availableExts = NULL;

    return found;
}
