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
 * @file extension.c
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
    if (!oriCheckLayerAvailability(layer)) {
        _ori_Warning("specified layer '%s' was not found", layer);
        return ORION_RETURN_STATUS_ERROR_NOT_FOUND;
    }

    state->instanceCreateInfo.enabledLayerCount++;

    state->instanceCreateInfo.enabledLayers = realloc(state->instanceCreateInfo.enabledLayers, state->instanceCreateInfo.enabledLayerCount * sizeof(char *));
    if (!state->instanceCreateInfo.enabledLayers) {
        _ori_ThrowError(ORERR_MEMORY_ERROR);
        return ORION_RETURN_STATUS_MEMORY_ERROR;
    }

    // here for readability
    char **lastElem = &state->instanceCreateInfo.enabledLayers[state->instanceCreateInfo.enabledLayerCount - 1];

    *lastElem = malloc(sizeof(char) * strlen(layer) + 1); // + 1 for null term
    strncpy(*lastElem, layer, strlen(layer) + 1);

    return ORION_RETURN_STATUS_OK;
}

/**
 * @brief Flag the specified Vulkan instance extension to be enabled when creating the state's instance with oriCreateStateInstance().
 *
 * For information regarding the difference between 'instance extensions' and 'device extensions' in Vulkan, you should see
 * <a href="https://stackoverflow.com/a/53050492/12980669">this</a> answer on StackOverflow.
 *
 * @param state the state the enable the extension on.
 * @param extension a UTF-8, null-terminated string holding the name of the desired extension.
 * @return true if the function executed successfully.
 * @return false if there was an error, such as if the desired instance extension was not supported (see the @ref group_Errors "debug output" for more information in this case)
 */
oriReturnStatus oriFlagInstanceExtensionEnabled(oriState *state, const char *extension) {
    // since the extension might be provided by a layer, and layers might not yet be specified, we don't
    // check for the extension's availability.
    // this will be done in oriCreateStateInstance() instead.

    state->instanceCreateInfo.enabledExtCount++;

    state->instanceCreateInfo.enabledExtensions = realloc(state->instanceCreateInfo.enabledExtensions, state->instanceCreateInfo.enabledExtCount * sizeof(char *));
    if (!state->instanceCreateInfo.enabledExtensions) {
        _ori_ThrowError(ORERR_MEMORY_ERROR);
        return ORION_RETURN_STATUS_MEMORY_ERROR;
    }

    // here for readability
    char **lastElem = &state->instanceCreateInfo.enabledExtensions[state->instanceCreateInfo.enabledExtCount - 1];

    *lastElem = malloc(sizeof(char) * strlen(extension) + 1); // + 1 for null term
    strncpy(*lastElem, extension, strlen(extension) + 1);

    // there technically isn't any need to have a return type but it is there for consistency with other functions
    return ORION_RETURN_STATUS_OK;
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
 * @ingroup group_Compat
 *
 */
bool oriCheckLayerAvailability(const char *layer) {
    unsigned int layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *availableLayers = malloc(layerCount * sizeof(VkLayerProperties));
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

    VkExtensionProperties *availableExts = malloc(extCount * sizeof(VkExtensionProperties));
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
