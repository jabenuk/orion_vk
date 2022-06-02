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
 * @file vk_debug.c
 * @author jack bennett
 * @brief Vulkan debugging
 *
 * @copyright Copyright (c) 2022
 *
 * This file defines functions involved with debugging Vulkan.
 *
 * Most of these functions require VK_EXT_debug_utils or similar
 * extensions to be enabled.
 *
 */

#include "orion.h"
#include "orion_structs.h"
#include "orion_helpers.h"
#include "orion_funcs.h"
#include "orion_codes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        CORE VULKAN API ABSTRACTIONS: DEBUGGING                   *****
// ============================================================================

/**
 * @brief Define the properties of messages you want to be displayed by the debug messenger of all instances created with the given state object.
 *
 * This function defines part of the creation info of <b>instance debug messengers</b>, which - if
 * @ref section_main_Config "ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS" is enabled and VK_EXT_debug_utils is specified - are automatically
 * created in oriCreateInstance().
 *
 * This function only has an effect if the VK_EXT_debug_utils extension has been specified. This doesn't necessarily have to be done before this
 * function is called (as long as it is before instance creation) but doing so @e will generate a warning nonetheless. The
 * @ref section_main_Config "ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS" flag must also be enabled - this @b does need to be done @b before
 * calling this function, and a warning will be generated otherwise.
 *
 * @note The message filters specified in this function are @b different to those specified in oriEnableLibDebugMessages().
 * Whilst those specified in that function are related to the Orion library, those specified here are related to the Vulkan API.
 *
 * @param state the state to modify.
 * @param severities (bitmask) severities of the messages to @b display. By default, no messages are displayed <i>(even if ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS is true)</i>
 * @param types (bitmask) types of the messages to @b display. By default, no messages are displayed <i>(even if ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS is true)</i>
 *
 * @sa <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_EXT_debug_utils.html">Vulkan Docs/VK_EXT_debug_utils</a>
 * @sa
 * <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessageSeverityFlagBitsEXT.html">Vulkan Docs/VkDebugUtilsMessageSeverityFlagBitsEXT</a>
 * @sa <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessageTypeFlagBitsEXT.html">Vulkan Docs/VkDebugUtilsMessageTypeFlagBitsEXT</a>
 *
 * @ingroup group_VkAbstractions_Core_Debugging
 *
 */
void oriSpecifyInstanceDebugMessages(oriState *state, VkDebugUtilsMessageSeverityFlagBitsEXT severities, VkDebugUtilsMessageTypeFlagBitsEXT types) {
    if (!state) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return;
    }

    if (!_orion.flags.createInstanceDebugMessengers) {
        _ori_Warning("%s", "instance debug filters were specified but CREATE_INSTANCE_DEBUG_MESSENGERS was not enabled at the time of calling oriSpecifyInstanceDebugMessages()");
    }

    if (!oriCheckInstanceExtensionEnabled(state, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) { // VK_EXT_DEBUG_UTILS_EXTENSION_NAME expands to VK_EXT_debug_utils
        _ori_Warning("instance debug filters were specified but the %s instance extension was not specified at the time of calling oriSpecifyInstanceDebugMessages()", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    state->instanceCreateInfo.dbgmsngrEnabledMessages.severities = severities;
    state->instanceCreateInfo.dbgmsngrEnabledMessages.types = types;
}

/**
 * @brief Create a Vulkan debug messenger that will be managed by @c state.
 *
 * The callback that will be used by this debug messenger is based on the Orion @ref section_Debugging_ErrorCallback "error callback"; that is to say,
 * a message will be generated by Orion with Vulkan-reported information, such as severity, message, type, etc, and will be reported to the Orion callback in
 * the @c message parameter. The code of the error will be @b 0x03, and the error name will be @b VULKAN_DEBUG_MESSENGER.
 *
 * The user pointer that will be reported to the callback is specified in oriSetErrorCallback(). You do not have to filter the messages yourself.
 *
 * @note The @c VK_EXT_debug_utils extension @b must be enabled; if not, @ref section_ErrorList_FunctionReturns "ORION_RETURN_STATUS_EXT_NOT_ENABLED" will be
 * returned, and @c debugMessengerPtr will be set to @b NULL.
 *
 * @param state the state object to which the resulting
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerEXT.html">VkDebugUtilsMessengerEXT</a> will be tied.
 * @param debugMessengerPtr a pointer to the structure to which the debug messenger will be returned.
 * @param severities (bitmask) severities of the messages to @b display.
 * @param types (bitmask) types of the messages to @b display.
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerEXT.html">Vulkan Docs/VkDebugUtilsMessengerEXT</a>
 *
 * @ingroup group_VkAbstractions_Core_Debugging
 *
 */
oriReturnStatus oriCreateDebugMessenger(oriState *state, VkInstance *instance, const void *ext, VkDebugUtilsMessengerEXT *debugMessengerPtr, VkDebugUtilsMessageSeverityFlagBitsEXT severities, VkDebugUtilsMessageTypeFlagBitsEXT types) {
    if (!state || !instance || !debugMessengerPtr) {
        _ori_ThrowError(ORERR_NULL_POINTER);
        return ORION_RETURN_STATUS_ERROR_NULL_POINTER;
    }

    // check extension enabled
    if (!oriCheckInstanceExtensionEnabled(state, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) { // VK_EXT_DEBUG_UTILS_EXTENSION_NAME expands to VK_EXT_debug_utils
        _ori_ThrowError(ORERR_EXT_NOT_ENABLED);
        *debugMessengerPtr = NULL;
        return ORION_RETURN_STATUS_EXT_NOT_ENABLED;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        ext,
        0,
        severities,
        types,
        _ori_VulkanDebugMessengerCallback,
        _orion.callbacks.errorCallbackUserData
    };

    // loading vkCreateDebugUtilsMessengerEXT() should always work here as the extension has already been checked for (above)
    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT");

    if (CreateDebugUtilsMessengerEXT(*instance, &createInfo, NULL, debugMessengerPtr)) {
        _ori_ThrowError(ORERR_VULKAN_RETURN_ERROR);
        return ORION_RETURN_STATUS_ERROR_VULKAN_ERROR;
    }

    // create a struct to point to both the debug messenger and the instance it was created for
    _ori_DebugUtilsMessengerEXT messengerStruct = {
        debugMessengerPtr,
        instance
    };
    // append this into state
    _ori_AppendOntoDArray(_ori_DebugUtilsMessengerEXT, state->arrays.debugMessengers, state->arrays.debugMessengersCount, messengerStruct);

    _ori_Notification("debug messenger created at %p for instance at %p", debugMessengerPtr, instance);

    return ORION_RETURN_STATUS_OK;
}
