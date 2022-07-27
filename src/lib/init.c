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
 * @file init.c
 * @author jack bennett
 * @brief Library initialisation, loading, and overall management
 *
 * @copyright Copyright (c) 2022 jack bennett
 *
 * This file defines functions involved with:
 *   - initialising the library
 *   - the overall termination of the library
 *   - configuring the library
 *
 */

#include "orion.h"
#include "orion_errors.h"
#include "orion_flags.h"
#include "orion_funcs.h"
#include "orion_structs.h"

#include <string.h>
#include <stdio.h>


// ============================================================================ //
// *****                     Private/internal systems                     ***** //
// ============================================================================ //

// we statically initialise callbacks in case they are called before oriInit().
_oriLibrary_t _orion = {
    NULL,
    .callbacks.debug.fun = _oriDefaultDebugCallback
};


// ============================================================================ //
// *****                  Orion library public interface                  ***** //
// ============================================================================ //


// ----[Orion library public interface]---------------------------------------- //
//                             Library management                               //

const oriReturnStatus_t oriInit(const unsigned int instanceCount, VkInstance *instanceOut, const VkInstanceCreateFlags instanceFlags, const unsigned int apiVersion, const char *applicationName, const unsigned int applicationVersion, const char *engineName, const unsigned int engineVersion, const unsigned int enabledLayerCount, const char **enabledLayers, const unsigned int enabledInstanceExtensionCount, const char **enabledInstanceExtensions, const void *instanceNext) {
    if (_orion.initialised) { // already initialised
        return ORION_RETURN_STATUS_SKIPPED;
    }
    if (!instanceOut || !instanceCount) { // no output variables given, or count is 0
        _oriWarning("all output variables NULL in call to %s", __func__);
        return ORION_RETURN_STATUS_NO_OUTPUT;
    }
    if (!instanceOut && instanceCount) { // no instances given but count is not 0
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }
    if (!enabledLayers && enabledLayerCount) { // no layers given but count is not 0
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }
    if (!enabledInstanceExtensions && enabledInstanceExtensionCount) { // no extensions given but count is not 0
        _oriError(ORIERR_NULL_POINTER, __func__);
        return ORION_RETURN_STATUS_NULL_POINTER;
    }

    // check if the given VkInstance pointer is already in the hash table of instances
    // this is really just a precautionary measure as there should only ever be one instance anyway. It could be omitted in the future if necessary.
    {
        _oriVkInstance_t *queryStructure;
        HASH_FIND_PTR(_orion.allocatees.vkInstances, instanceOut, queryStructure);
        if (queryStructure) { // uthash will have set queryStructure to NULL if it wasn't found so we can rely on this check
            _oriWarning("orion already allocated instance at %p (%s)", instanceOut, __func__);
            return ORION_RETURN_STATUS_SKIPPED;
        }
    }

    // create application info struct
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL, // "pNext must be NULL" - from VkApplicationInfo(3)
        .pApplicationName = applicationName,
        .applicationVersion = applicationVersion,
        .pEngineName = engineName,
        .engineVersion = engineVersion,
        .apiVersion = apiVersion
    };

    // create instance create-info struct
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = instanceNext;
    createInfo.flags = instanceFlags;
    createInfo.pApplicationInfo = &appInfo;

#   ifdef __oridebug
        // debug message will be logged at the end of the function if successful
        char logstr[MAX_LOG_LEN];
        memset(logstr, 0, MAX_LOG_LEN);
        snprintf(logstr, MAX_LOG_LEN, "%d instance%s created into %p (%s)", instanceCount, (instanceCount == 1) ? "" : "s", instanceOut, __func__);
#   endif

    // static arrays that will hold the compatible layers and extensions
    // we are storing these in the primary function scope because they are referenced by vkCreateInstance() and so must be preserved until then.
    // as each specified layer/extension is validated, it is added to the respective array here (assuming it was found to be compatible.)
    const char *actualEnabledLayers[enabledLayerCount];
    const char *actualEnabledExts[enabledInstanceExtensionCount];
    unsigned int actualEnabledLayerCount = 0;
    unsigned int actualEnabledExtCount = 0;

    // specify layers to be enabled
    if (enabledLayerCount && enabledLayers) {
#       ifdef __oridebug
            // string will be concatenated onto logstr
            char logstr_layerlist[MAX_LOG_LEN];
            memset(logstr_layerlist, 0, MAX_LOG_LEN);
#       endif

        // check that each layer is compatible
        for (unsigned int i = 0; i < enabledLayerCount; i++) {
            if (oriCheckLayerAvailability(enabledLayers[i])) {
                // add to 'actual' array of layers
                actualEnabledLayers[i] = enabledLayers[i];
                actualEnabledLayerCount++;

#               ifdef __oridebug
                    char s[MAX_LOG_LEN];
                    snprintf(s, MAX_LOG_LEN, "\n\t\t[%u] name '%s'", i, enabledLayers[i]);
                    strncat(logstr_layerlist, s, MAX_LOG_LEN);
#               endif
            }
#           ifdef __oridebug
                else {
                    // print warning stating that the layer was not available
                    _oriWarning("layer %s not provided by Vulkan implementation", enabledLayers[i]);
                }
#           endif
        }

        // pass filtered layer array to Vulkan
        if (actualEnabledLayerCount) {
            createInfo.enabledLayerCount = actualEnabledLayerCount;
            createInfo.ppEnabledLayerNames = (const char *const *) actualEnabledLayers;

#           ifdef __oridebug
                // concatenate onto logstr
                char s[MAX_LOG_LEN];
                snprintf(s, MAX_LOG_LEN, "\n\t%u layers enabled:", actualEnabledLayerCount);
                strncat(logstr, s, MAX_LOG_LEN);

                strncat(logstr, logstr_layerlist, MAX_LOG_LEN);
#           endif
        }
    }

    // specify extensions to be enabled
    if (enabledInstanceExtensionCount && enabledInstanceExtensions) {
#       ifdef __oridebug
            // string will be concatenated onto logstr
            char logstr_extlist[MAX_LOG_LEN];
            memset(logstr_extlist, 0, MAX_LOG_LEN);
#       endif

        // iterate through extensions
        for (unsigned int i = 0; i < enabledInstanceExtensionCount; i++) {
            // we use this design instead of continuing so the debug message is ensured to be concatenated out if __oridebug is enabled*
            bool provided = false;

            // check if the current extension is provided by the Vulkan implementation
            if (oriCheckInstanceExtensionAvailability(enabledInstanceExtensions[i], NULL)) {
                actualEnabledExts[i] = enabledInstanceExtensions[i];
                actualEnabledExtCount++;
                provided = true;
            }

            // iterate through layers if not yet provided
            if (!provided) {
                for (unsigned int j = 0; j < actualEnabledLayerCount; j++) {
                    // check if the current extension is provided by the current layer
                    if (oriCheckInstanceExtensionAvailability(enabledInstanceExtensions[i], actualEnabledLayers[j])) {
                        actualEnabledExts[i] = enabledInstanceExtensions[i];
                        actualEnabledExtCount++;
                        provided = true;
                        break;
                    }
                }
            }

            // *this is where the extension name is concatenated (if the current extension turned out to be available)
#           ifdef __oridebug
                if (provided) {
                    char s[MAX_LOG_LEN];
                    snprintf(s, MAX_LOG_LEN, "\n\t\t[%u] name '%s'", i, enabledInstanceExtensions[i]);
                    strncat(logstr_extlist, s, MAX_LOG_LEN);
                } else {
                    // print warning stating that the extension was not available
                    _oriWarning("instance extension %s not provided by Vulkan implementation or any layers", enabledInstanceExtensions[i]);
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
                snprintf(s, MAX_LOG_LEN, "\n\t%u instance extensions enabled:", actualEnabledExtCount);
                strncat(logstr, s, MAX_LOG_LEN);

                strncat(logstr, logstr_extlist, MAX_LOG_LEN);
#           endif
        }
    }

    for (unsigned int i = 0; i < instanceCount; i++) {
        if (vkCreateInstance(&createInfo, _orion.callbacks.vulkanAllocators, &instanceOut[i])) {
            _oriError(ORIERR_INSTANCE_CREATION_FAIL, __func__);
            return ORION_RETURN_STATUS_ERROR;
        }
    }

    // create a wrapper for the instance which can be stored in _orion
    _oriVkInstance_t *wrapper = malloc(sizeof(_oriVkInstance_t));
    if (!wrapper) {
        _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }
    wrapper->handle = instanceOut;

    // store the enabled layers
    wrapper->layerCount = actualEnabledLayerCount;
    wrapper->layers = malloc(sizeof(const char *) * actualEnabledLayerCount);
    if (!wrapper->layers) {
        _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    for (unsigned int i = 0; i < actualEnabledLayerCount; i++) {
        // since the array of enabled layers here is stack-allocated, we need to malloc each string in the wrapper object
        wrapper->layers[i] = malloc(sizeof(char) * (1 + strlen(actualEnabledLayers[i]))); // we add 1 for the null terminator
        if (!wrapper->layers[i]) {
            _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
            return ORION_RETURN_STATUS_ERROR;
        }

        strcpy(wrapper->layers[i], actualEnabledLayers[i]);
    }

    // store the enabled instance extensions
    wrapper->extensionCount = actualEnabledExtCount;
    wrapper->extensions = malloc(sizeof(const char *) * actualEnabledExtCount);
    if (!wrapper->extensions) {
        _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
        return ORION_RETURN_STATUS_ERROR;
    }

    for (unsigned int i = 0; i < actualEnabledExtCount; i++) {
        wrapper->extensions[i] = malloc(sizeof(char) * (1 + strlen(actualEnabledExts[i]))); // we add 1 for the null terminator
        if (!wrapper->extensions[i]) {
            _oriFatalError(ORIFERR_MEMORY_ERROR, __func__);
            return ORION_RETURN_STATUS_ERROR;
        }

        strcpy(wrapper->extensions[i], actualEnabledExts[i]);
    }

    // internally store the wrapper
    HASH_ADD_PTR(_orion.allocatees.vkInstances, handle, wrapper);

#   ifdef __oridebug
        _oriNotification(logstr);
#   endif

    // the library has been initialised
    _orion.initialised = true;
    return ORION_RETURN_STATUS_OK;
}

const oriReturnStatus_t oriTerminate() {
#   ifdef __oridebug
        _oriNotification("lib term called (%s)", __func__);
#   endif

    // destroy instance(s)
    {
        // use buffer for deletion-safe iteration
        _oriVkInstance_t *cur, *buffer;
        HASH_ITER(hh, _orion.allocatees.vkInstances, cur, buffer) {
            // destroy vulkan object
            if (cur->handle) {
                vkDestroyInstance(*cur->handle, _orion.callbacks.vulkanAllocators);
            }

            // free array of layers
            for (unsigned int i = 0; i < cur->layerCount; i++) {
                free(cur->layers[i]);
                cur->layers[i] = NULL;
            }
            free(cur->layers);
            cur->layers = NULL;

            // free array of instance extensions
            for (unsigned int i = 0; i < cur->extensionCount; i++) {
                free(cur->extensions[i]);
                cur->extensions[i] = NULL;
            }
            free(cur->extensions);
            cur->extensions = NULL;

            // free each wrapper struct
            HASH_DEL(_orion.allocatees.vkInstances, cur);
            free(cur);
            cur = NULL;
        }
    }

    // the library can be re-initialised after this point
    memset(&_orion, 0, sizeof(_orion));
    _orion.initialised = false;

    // set debug callback as it (maybe?) could be called after this point despite _orion having been cleared
    _orion.callbacks.debug.fun = _oriDefaultDebugCallback;

    return ORION_RETURN_STATUS_OK;
}

const oriReturnStatus_t oriSetVulkanAllocators(VkAllocationCallbacks *callbacks) {
#   ifdef __oridebug
        _oriLog("vulkan allocators updated to loc %p (%s)", callbacks, __func__);
#   endif

    _orion.callbacks.vulkanAllocators = callbacks;
    return ORION_RETURN_STATUS_OK;
}

const VkAllocationCallbacks *oriGetVulkanAllocators() {
    return _orion.callbacks.vulkanAllocators;
}
