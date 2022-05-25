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



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        STATE                                                     *****
// ============================================================================

_ori_LibState _orion = { NULL };

// initialise library flags to their default values
_ori_LibFlags _orionflags = {};


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

    char logstr[768];
    snprintf(logstr, 768, "vulkan instance created and will be managed by state object at location %p...", state);
    // don't add a new line unless necessary
    if (state->instanceCreateInfo.enabledLayerListHead || state->instanceCreateInfo.enabledExtListHead) {
        strncat(logstr, "\n", 2); // more than the source length to get rid of GCC warnings
    }

    // the support of these layers was already checked in oriFlagLayerEnabled().
    if (state->instanceCreateInfo.enabledLayerListHead) {
        unsigned int i = 0;

        char logstr_layer[768];
        snprintf(logstr_layer, 768, "\t...layers enabled for this instance:\n");

        // traverse through the layer linked list, adding each element to the layerNames array
        // this array is then passed into the createInfo structure
        _ori_StrList *cur = state->instanceCreateInfo.enabledLayerListHead;
        while (cur) {
            layerNames[i] = malloc(sizeof(char) * strlen(cur->data) + 1); // + 1 for null term.
            strncpy(layerNames[i], cur->data, strlen(cur->data) + 1);

            char s[768];
            snprintf(s, 768, "\t\t- name '%s'\n", layerNames[i]);
            strncat(logstr_layer, s, 767); // GCC wants strncat to have one less than the length of dest, so instead of 768, we specify 767.

            cur = cur->next;
            i++;
        }

        strncat(logstr, logstr_layer, 767);

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

        char logstr_extension[768];
        snprintf(logstr_extension, 768, "\t...instance extensions enabled for this instance:\n");

        cur = state->instanceCreateInfo.enabledExtListHead;
        while (cur) {
            extNames[i] = malloc(sizeof(char) * strlen(cur->data) + 1); // + 1 for null term.
            strncpy(extNames[i], cur->data, strlen(cur->data) + 1);

            char s[768];
            snprintf(s, 768, "\t\t- name '%s'", extNames[i]);
            if (i < createInfo.enabledExtensionCount - 1) strncat(s, "\n", 767); // we don't want a newline at the end of the string
            strncat(logstr_extension, s, 767);

            cur = cur->next;
            i++;
        }

        strncat(logstr, logstr_extension, 767);

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
            (*cur)->data = NULL;

            free(*cur); // *cur will be NULL by the end of this while loop
            *cur = next;
        }
        state->instanceCreateInfo.enabledLayerCount = 0;
    }
    {
        _ori_StrList **cur = &state->instanceCreateInfo.enabledExtListHead;
        while (*cur) {
            _ori_StrList *next = (*cur)->next;
            free((*cur)->data); // since the data was malloc'd as well we need to explicitly free it too
            (*cur)->data = NULL;

            free(*cur); // *cur will be NULL by the end of this while loop
            *cur = next;
        }
        state->instanceCreateInfo.enabledExtCount = 0;
    }

    _ori_Notification(logstr, 0);

    // also free arrays of malloc'd strings
    // this is done after creating the instance as they are referenced by the create info
    for (unsigned int i = 0; i < createInfo.enabledLayerCount; i++) {
        free(layerNames[i]);
        layerNames[i] = NULL;
    }
    for (unsigned int i = 0; i < createInfo.enabledExtensionCount; i++) {
        free(extNames[i]);
        extNames[i] = NULL;
    }

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
oriState *oriCreateState(unsigned int apiVersion) {
    oriState *r = malloc(sizeof(oriState));

    // init to ensure everything (most importantly linked list heads) start at NULL
    memset(r, 0, sizeof(oriState));

    r->apiVersion = apiVersion;

    _ori_DebugLog("state object created at API version %d.%d.%d", VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));

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
 * @param name NULL, or a string containing the name of the application.
 * @param version the version of the application.
 * @param engineName NULL, or a string containing the name of the engine used to create the application.
 * @param engineVersion the version of the engine used to to create the application.
 * @param apiVersion the highest version of Vulkan that the application is to use.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html">Vulkan Docs/VkApplicationInfo</a>
 *
 * @ingroup group_Meta
 *
 */
void oriDefineStateApplicationInfo(oriState *state, const void *ext, const char *name, unsigned int version, const char *engineName, unsigned int engineVersion) {
    // using a compound literal should (?) be far less expensive than constantly dereferencing state to redefine the properties separately.
    state->appInfo = (VkApplicationInfo) {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        ext,
        name,
        version,
        engineName,
        engineVersion,
        state->apiVersion
    };

    _ori_DebugLog(
        "application info of state object at %p updated:\n"
        "\tname : %s\n"
        "\tversion : %d.%d.%d\n"
        "\tengine name : %s\n"
        "\tengine version : %d.%d.%d\n"
        "\textensive structure : at %p",
        state, name,
        VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version),
        engineName,
        VK_VERSION_MAJOR(engineVersion), VK_VERSION_MINOR(engineVersion), VK_VERSION_PATCH(engineVersion),
        ext
    );
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
            _ori_Warning("An invalid flag was given to oriSetFlag(); nothing was updated.", 0);
            return ORION_RETURN_STATUS_ERROR_INVALID_ENUM;
    }

    _ori_DebugLog("flag %s set to %d", flagstr, val);
    return ORION_RETURN_STATUS_OK;
}
