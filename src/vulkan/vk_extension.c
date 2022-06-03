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
 * @file vk_extension.c
 * @author jack bennett
 * @brief Vulkan extensions and feature loading
 *
 * @copyright Copyright (c) 2022
 *
 * This file contains functions broadly related to the use of Vulkan API features.
 *
 * This isn't specifically extensions, but rather instance + device extensions as well as layers.
 * ... and anything else Vulkan-related that isn't in the Core API.
 *
 */

#include "orion.h"
#include "orion_structs.h"
#include "orion_codes.h"
#include "orion_helpers.h"
#include "orion_funcs.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        FEATURE LOADING AND ENABLING                              *****
// ============================================================================

/**
 * @brief Enable the specified Vulkan layer.
 *
 * @param layer a UTF-8, null-terminated string holding the name of the desired layer.
 * @return true if the function executed successfully.
 * @return false if there was an error, such as if the desired layer was not supported (see the @ref group_Errors "debug output" for more information in this case)
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
oriReturnStatus oriFlagLayerEnabled(oriState *state, const char *layer) {
    if (!state) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return ORION_RETURN_STATUS_ERROR_NULL_POINTER;
    }

    if (!oriCheckLayerAvailability(layer)) {
#       ifdef __oridebug
            _ori_Warning("specified layer '%s' was not found", layer);
#       endif

        return ORION_RETURN_STATUS_ERROR_NOT_FOUND;
    }

    _ori_AppendOntoDArray(char *, state->instanceCreateInfo.enabledLayers, state->instanceCreateInfo.enabledLayerCount, (char *) layer);

    return ORION_RETURN_STATUS_OK;
}

/**
 * @brief Flag the specified Vulkan instance extension to be enabled when creating the state's instance with oriCreateInstance().
 *
 * For information regarding the difference between 'instance extensions' and 'device extensions' in Vulkan, you should see
 * <a href="https://stackoverflow.com/a/53050492/12980669">this</a> answer on StackOverflow.
 *
 * @param state the state the enable the extension on.
 * @param extension a UTF-8, null-terminated string holding the name of the desired extension.
 * @return true if the function executed successfully.
 * @return false if there was an error, such as if the desired instance extension was not supported (see the @ref group_Errors "debug output" for more information in this case)
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
oriReturnStatus oriFlagInstanceExtensionEnabled(oriState *state, const char *extension) {
    if (!state) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return ORION_RETURN_STATUS_ERROR_NULL_POINTER;
    }

    // since the extension might be provided by a layer, and layers might not yet be specified, we don't
    // check for the extension's availability.
    // this would be done in oriPruneInstanceExtensions() instead.

    _ori_AppendOntoDArray(char *, state->instanceCreateInfo.enabledExtensions, state->instanceCreateInfo.enabledExtCount, (char *) extension);

    // there technically isn't any need to have a return type but it is there for consistency with other functions
    return ORION_RETURN_STATUS_OK;
}

/**
 * @brief Validate any instance extensions in @c state against now-specified layers.
 *
 * This function validates and removes any unavailable or non-existant instance extensions that have been specified for the
 * oriState object @c state.
 *
 * This allows for you to specify instance extensions @b before the layers that provide them - essentially, Vulkan instance extensions are either
 * provided by the implementation or a layer.
 *
 * This function is implicitly called in oriCreateInstance() in case you forget.
 *
 * @warning If you specify instance extensions that are provided by a layer, and you call this function before specifying said layer, these instance
 * extensions will be @b removed from the state's internal list, and will @b not be enabled, even if you specify the layer after this function.
 *
 * @param state the state to validate.
 * @return true if any extensions have been removed from the state's list.
 * @return false if no extensions have been removed.
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriPruneInstanceExtensions(oriState *state) {
    if (!state) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return false;
    }

    bool r = false;

    for (unsigned int i = 0; i < state->instanceCreateInfo.enabledExtCount; i++) {
        // store current extension on stack to reduce dereferences
        const char *curext = state->instanceCreateInfo.enabledExtensions[i];

        // check if the extension is already provided by the implementation
        if (oriCheckInstanceExtensionAvailability(curext, NULL)) {
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

        // if this is reached, the extension is NOT provided by the implementation.
        // but, the extension IS provided by a layer.
        // so we can safely move on to the next extension.
        if (_providedByALayer) {
            continue;
        }

        // if this is reached, then we know the extension is not provided by the implementation and there are no layers that could provide it
        // so we know the extension is not available
        r = true;

#       ifdef __oridebug
            _ori_Warning("specified instance extension '%s' was not found, removed from list of state at %p", curext, state);
#       endif

        // remove the extension from the list
        _ori_RemoveFromDArray(state->instanceCreateInfo.enabledExtensions, state->instanceCreateInfo.enabledExtCount, i);
    }

    return r;
}



// ============================================================================
// *****        COMPATIBILITY CHECKS                                      *****
// ============================================================================

/**
 * @brief Return the availability of the specified Vulkan layer.
 *
 * @param layer a UTF-8, null-terminated string holding the name of the desired layer.
 * @return true if the layer is available
 * @return false if the layer is not available
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriCheckLayerAvailability(const char *layer) {
    unsigned int layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *availableLayers = calloc(layerCount, sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    bool found = false;

    for (unsigned int i = 0; i < layerCount; i++) {
        if (!strcmp(availableLayers[i].layerName, layer)) {
            // return at end of function to ensure availableLayers is freed
            found = true;
            break;
        }
    }

    free(availableLayers);
    availableLayers = NULL;

    return found;
}

/**
 * @brief Check if @c layer is enabled in the state object @c state.
 *
 * The layer does not have to be actually 'enabled' yet; the function will still return true if the layer has at least been
 * @b specified (with oriFlagLayerEnabled()) - this means no instances have to have been created with @c state for the function
 * to work properly.
 *
 * You don't need to worry about validating the list of layers; the availability of the specified layers is immediately checked
 * in oriFlagLayerEnabled().
 *
 * @param state the oriState object to query.
 * @param layer the layer to search for.
 * @return true if the layer has been enabled.
 * @return false if the layer has not been enabled.
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriCheckLayerEnabled(oriState *state, const char *layer) {
    if (!state) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return false;
    }

    for (unsigned int i = 0; i < state->instanceCreateInfo.enabledLayerCount; i++) {
        if (!strcmp(state->instanceCreateInfo.enabledLayers[i], layer)) {
            return true;
        }
    }

    // layer not found in array
    return false;
}

/**
 * @brief Return the availability of the specified Vulkan instance extension.
 *
 * The @c layer parameter is equivalent to the @c pLayerName parameter in the
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkEnumerateInstanceExtensionProperties.html">VkEnumerateInstanceExtensionProperties</a>
 * Vulkan function. This is described in the Specification as the following:
 *  > When pLayerName parameter is NULL, only extensions provided by the Vulkan implementation or by implicitly enabled layers are returned. When pLayerName is the
 *  > name of a layer, the instance extensions provided by that layer are returned.
 *
 * @param extension a UTF-8, null-terminated string holding the name of the desired instance extension.
 * @param layer NULL or a UTF-8 null-terminated string holding the name of the layer to query the extension from.
 * @return true if the instance extension is available
 * @return false if the instance extension is not available
 *
 * @sa oriFlagInstanceExtensionEnabled()
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriCheckInstanceExtensionAvailability(const char *extension, const char *layer) {
    unsigned int extCount = 0;
    vkEnumerateInstanceExtensionProperties(layer, &extCount, NULL);

    VkExtensionProperties *availableExts = calloc(extCount, sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(layer, &extCount, availableExts);

    bool found = false;

    for (unsigned int i = 0; i < extCount; i++) {
        if (!strcmp(availableExts[i].extensionName, extension)) {
            // return at end of function to ensure availableExts is freed
            found = true;
            break;
        }
    }

    free(availableExts);
    availableExts = NULL;

    return found;
}

/**
 * @brief Check if the instance extension @c extension is enabled in the state object @c state.
 *
 * Similar to in oriCheckLayerEnabled(), the extension does not have to be actually 'enabled' yet; the function will
 * still return true if it has at least been @b specified (with oriFlagInstanceExtensionEnabled()) - this means no instances
 * have to have been created with @c state for the function to work properly.
 *
 * @b Unlike with layers, however, the validation of the specified instance extensions is not checked outside of
 * oriPruneInstanceExtensions() (which is implicitly called in oriCreateInstance()). If you are calling this function @e before
 * oriCreateInstance(), you should prune the instance extensions first to avoid false positives on invalid extensions. Note, though,
 * that any extensions that are provided by layers not yet specified (even if said layers are specified later) will be
 * @b removed from the state's list of instance extensions. See oriPruneInstanceExtensions() for more.
 *
 * @param state the state object to query.
 * @param extension the instance extension to search for.
 * @return true
 * @return false
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriCheckInstanceExtensionEnabled(oriState *state, const char *extension) {
    if (!state) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return false;
    }

    for (unsigned int i = 0; i < state->instanceCreateInfo.enabledExtCount; i++) {
        if (!strcmp(state->instanceCreateInfo.enabledExtensions[i], extension)) {
            return true;
        }
    }

    // extension not found in array
    return false;
}
