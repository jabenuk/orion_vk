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

#include "orion.h"
#include "internal.h"

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
oriReturnStatus oriCreateStateVkInstance(oriState *state, VkInstance *instancePtr) {
    oriReturnStatus r = ORION_RETURN_STATUS_OK;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    createInfo.pApplicationInfo = &state->appInfo;

    // specify layers to be enabled
    createInfo.enabledLayerCount = state->instanceCreateInfo.enabledLayerCount;

    // this is stored here as it needs to be freed AFTER instance creation
    char *layerNames[createInfo.enabledLayerCount]; // we access from createInfo instead of state as no dereference is necessary

    // the support of these layers was already checked in oriFlagLayerEnabled().
    if (state->instanceCreateInfo.enabledLayerListHead) {
        unsigned int i = 0;

        char logstr[256] = "vulkan layers enabled:\n";

        // traverse through the layer linked list, adding each element to the layerNames array
        // this array is then passed into the createInfo structure
        _ori_StrList *cur = state->instanceCreateInfo.enabledLayerListHead;
        while (cur) {
            layerNames[i] = malloc(sizeof(char) * strlen(cur->data) + 1); // + 1 for null term.
            strncpy(layerNames[i], cur->data, strlen(cur->data) + 1);

            strncat(logstr, "\t - name '", 255);
            strncat(logstr, layerNames[i], 255);
            strncat(logstr, "'", 255);
            if (i < createInfo.enabledLayerCount - 1) strncat(logstr, "'\n", 255);

            cur = cur->next;
            i++;
        }

        _ori_Notification(logstr, 0);

        createInfo.ppEnabledLayerNames = (const char *const *) layerNames;
    } else {
        createInfo.ppEnabledLayerNames = NULL;
    }

    // now for extensions
    createInfo.enabledExtensionCount = state->instanceCreateInfo.enabledExtCount;

    // stored in this scope for same reasons as layerNames
    char *extNames[createInfo.enabledExtensionCount]; // we access from createInfo instead of state as no dereference is necessary

    // unlike layers, the support of the specified extensions has not yet been checked
    // this is because the layers that provide some extensions might not yet have been specified
    // but we can now check for the support of each extension, removing the extension from the list if it is not supported.
    // obviously, there will be a big warning in the debug output if any desired extensions are not loaded + false will be returned at the end.
    if (state->instanceCreateInfo.enabledExtListHead) {
        // we first check the availability of each specified extension
        _ori_StrList *cur = state->instanceCreateInfo.enabledExtListHead;
        _ori_StrList *prev = NULL, *next = NULL; // these are here for removing any extensions from the list

        while (cur) {
            next = cur->next;

            // first check if the extension is already provided by the implementation
            if (oriCheckInstanceExtensionAvailability(cur->data, NULL)) {
                // make sure prev and cur are set in case the next extension isn't found
                prev = cur;
                cur = cur->next;
                continue;
            }

            // otherwise:
            _ori_StrList *curlayer = state->instanceCreateInfo.enabledLayerListHead;
            bool _found = false;

            if (curlayer) {
                // if there are any layers, traverse through the list of them and check if they provide the given extension
                // set _found to true on the first one that does
                while (curlayer && !_found) {
                    if (oriCheckInstanceExtensionAvailability(cur->data, curlayer->data)) {
                        _found = true;
                    }

                    curlayer = curlayer->next;
                }
            }

            if (!_found) {
                // if this is reached, then we know the extension is not provided by the implementation and there are no layers that could provide it
                // so we know the extension is not available

                _ori_Warning("specified instance extension `%s` was not found", cur->data);
                r = ORION_RETURN_STATUS_ERROR_NOT_FOUND;

                // remove the extension from the list
                if (prev && next)       prev->next = next;
                else if (prev && !next) prev->next = NULL;
                else if (!prev && next) state->instanceCreateInfo.enabledExtListHead = next;
                else                    state->instanceCreateInfo.enabledExtListHead = NULL;

                // decrease counter by one
                // (the state variable doesn't matter anymore so we just change the create info count)
                createInfo.enabledExtensionCount--;
            }

            prev = cur;
            cur = cur->next;
        }

        // any unsupported extensions have now been removed from the list, so we can traverse through it again
        // this time, we add each extension to an array which is then passed to the create info struct.
        unsigned int i = 0;

        char logstr[256] = "vulkan instance extensions enabled:\n";

        cur = state->instanceCreateInfo.enabledExtListHead;
        while (cur) {
            extNames[i] = malloc(sizeof(char) * strlen(cur->data) + 1); // + 1 for null term.
            strncpy(extNames[i], cur->data, strlen(cur->data) + 1);

            strncat(logstr, "\t - name '", 255);
            strncat(logstr, extNames[i], 255);
            strncat(logstr, "'", 255);
            if (i < createInfo.enabledExtensionCount - 1) strncat(logstr, "\n", 255);

            cur = cur->next;
            i++;
        }

        _ori_Notification(logstr, 0);

        createInfo.ppEnabledExtensionNames = (const char *const *) extNames;
    } else {
        createInfo.ppEnabledExtensionNames = NULL;
    }

    if (vkCreateInstance(&createInfo, NULL, instancePtr)) {
        r = ORION_RETURN_STATUS_ERROR_VULKAN_ERROR;
        _ori_ThrowError(ORERR_VULKAN_RETURN_ERROR);
    }
    state->instance = instancePtr;

    // we no longer need the data used for the create info struct so we can free it all
    {
        _ori_StrList **cur = &state->instanceCreateInfo.enabledLayerListHead;
        while (*cur) {
            _ori_StrList *next = (*cur)->next;
            free((*cur)->data); // since the data was malloc'd as well we need to explicitly free it too
            free(*cur);
            *cur = next;
        }
        state->instanceCreateInfo.enabledLayerCount = 0;
    }
    {
        _ori_StrList **cur = &state->instanceCreateInfo.enabledExtListHead;
        while (*cur) {
            _ori_StrList *next = (*cur)->next;
            free((*cur)->data); // since the data was malloc'd as well we need to explicitly free it too
            free(*cur);
            *cur = next;
        }
        state->instanceCreateInfo.enabledExtCount = 0;
    }

    // also free arrays of malloc'd strings
    // this is done after creating the instance as they are referenced by the create info
    for (unsigned int i = 0; i < createInfo.enabledLayerCount; i++) {
        free(layerNames[i]);
    }
    for (unsigned int i = 0; i < createInfo.enabledExtensionCount; i++) {
        free(extNames[i]);
    }

    return r;
}



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

    _ori_StrList *l = malloc(sizeof(_ori_StrList));
    l->data = malloc(sizeof(char) * strlen(layer) + 1); // + 1 for null term
    strncpy(l->data, layer, strlen(layer) + 1);

    l->next = state->instanceCreateInfo.enabledLayerListHead;
    state->instanceCreateInfo.enabledLayerListHead = l;

    return ORION_RETURN_STATUS_OK;
}

/**
 * @brief Flag the specified Vulkan instance extension to be enabled when creating the state's instance with oriCreateStateVkInstance().
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
    // this will be done in oriCreateStateVkInstance() instead.

    state->instanceCreateInfo.enabledExtCount++;

    _ori_StrList *e = malloc(sizeof(_ori_StrList));
    e->data = malloc(sizeof(char) * strlen(extension) + 1); // + 1 for null term.
    strncpy(e->data, extension, strlen(extension) + 1);

    e->next = state->instanceCreateInfo.enabledExtListHead;
    state->instanceCreateInfo.enabledExtListHead = e;

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
    return found;
}
