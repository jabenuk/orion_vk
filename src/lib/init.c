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
 * @brief Library initialisation, loading, and overall management
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
#include "orion_structs.h"
#include "orion_helpers.h"
#include "orion_funcs.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================

// initialisation of global internal variables
_ori_Lib _orion = {};



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================



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
    _ori_RemoveArrayDuplicates(state->arrays.instances, state->arrays.instancesCount);

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
