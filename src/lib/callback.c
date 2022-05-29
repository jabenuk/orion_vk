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
 * @file callback.c
 * @author jack bennett
 * @brief Default and internal callbacks
 *
 * @copyright Copyright (c) 2022
 *
 * This file contains definitions for callbacks used both internally within the
 * Orion library and as default options for user-set callbacks.
 *
 */

#include "orion.h"
#include "orion_structs.h"

#include <string.h>
#include <stdio.h>



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        DEFAULT CALLBACKS                                         *****
// ============================================================================

// declared in headers/orion_funcs.h
// default error callback (will be set in oriDefaultCallbacks())
void _ori_DefaultErrorCallback(const char *name, unsigned int code, const char *message, oriErrorSeverityBit severity, void *pointer) {
    // if the error message has been reported by a Vulkan debug messenger, we don't want to bother printing this other
    // stuff, as it is irrelevant.
    if (!strcmp(name, "VULKAN_DEBUG_MESSENGER")) {
        printf("[orion] (VULKAN_DEBUG_MESSENGER) %s\n", message);
        return;
    }

    char severitystr[15];

    switch (severity) {
        case ORION_ERROR_SEVERITY_WARNING_BIT:
            strncpy(severitystr, "WARNING", 15);
            break;
        case ORION_ERROR_SEVERITY_ERROR_BIT:
            strncpy(severitystr, "ERROR", 15);
            break;
        case ORION_ERROR_SEVERITY_FATAL_BIT:
            strncpy(severitystr, "FATAL!", 15);
            break;
        default:
            // the severity will not be printed in logs and notifications.
            break;
    }

    // style messages differently if they aren't indicating any problems
    // (errors and fatal errors are normally standardised, with codes and such, but messages and warnings are not)
    switch (severity) {
        case ORION_ERROR_SEVERITY_VERBOSE_BIT:
        case ORION_ERROR_SEVERITY_NOTIF_BIT:
            printf("[orion] %s\n", message);
            break;
        case ORION_ERROR_SEVERITY_WARNING_BIT:
            printf("[orion] (%s) %s\n", severitystr, message);
            break;
        default:
            printf("[orion] (%s) %s (code 0x%02X): \"%s\"\n", severitystr, name, code, message);
            break;
    }
}

// declared in headers/orion_funcs.h
// this is always used for any debug messengers created under an Orion state
VKAPI_ATTR VkBool32 VKAPI_CALL _ori_VulkanDebugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagBitsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackdata,
    void *userdata
) {
    char msg[1024];
    memset(msg, 0, 1024);

    // convert the severity to a string
    char severityStr[15];
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            snprintf(severityStr, 15, "VERBOSE");
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            snprintf(severityStr, 15, "NOTIFICATION");
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            snprintf(severityStr, 15, "WARNING");
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        default:
            snprintf(severityStr, 15, "ERROR");
            break;
    }

    // verbose and info messages normally rely on each other for context (and are normally (although not always) one liners), so
    // getting rid of the additional fluff makes the output a lot clearer
    if (
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
    ) {
        snprintf(msg, 1024, "%s >> %s", severityStr, callbackdata->pMessage);

        // this is code reuse from later in the function, but...
        //  1. it's a small code snippet
        //  2. copying this here and returning early is less expensive than doing all the string
        //       manipulation only to discard it and just report pMessage instead of msg.

        // since we aren't calling from _ori_ThrowError(), we need to check for errorCallback's validity first
        if (!_orion.callbacks.errorCallback) {
            _orion.callbacks.errorCallback = _ori_DefaultErrorCallback;
        }

        _orion.callbacks.errorCallback("VULKAN_DEBUG_MESSENGER", 0x03, msg, ORION_ERROR_SEVERITY_ERROR_BIT, userdata);

        return VK_FALSE;
    }

    // convert the type to string
    char typeStr[15];
    switch (type) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        default:
            snprintf(typeStr, 15, "GENERAL");
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            snprintf(typeStr, 15, "VALIDATION");
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            snprintf(typeStr, 15, "PERFORMANCE");
            break;
    }

    // add basic error info
    snprintf(msg, 1024,
        "vulkan reported debug message, details are described below:\n"
        "\tseverity %s, type %s\n"
        "\t\tMESSAGE BEGIN\n\t\t\t%s\n\t\tMESSAGE END",
        severityStr, typeStr,
        callbackdata->pMessage
    );

    // append queue info
    {
        strncat(msg, "\n\tamount of active items in current VkQueue: ", 1023);

        char amt[8];
        snprintf(amt, 8, "%d", callbackdata->queueLabelCount);
        strncat(msg, amt, 1023);

        for (unsigned int i = 0; i < callbackdata->queueLabelCount; i++) {
            if (callbackdata->pQueueLabels[i].pLabelName) {
                strncat(msg, "\n\t\tlabel: ", 1023);
                strncat(msg, callbackdata->pQueueLabels[i].pLabelName, 1023);
            }
        }
    }

    // append command buffer info
    {
        strncat(msg, "\n\tamount of active items in current VkCommandBuffer: ", 1023);

        char amt[8];
        snprintf(amt, 8, "%d", callbackdata->cmdBufLabelCount);
        strncat(msg, amt, 1023);

        for (unsigned int i = 0; i < callbackdata->cmdBufLabelCount; i++) {
            if (callbackdata->pCmdBufLabels[i].pLabelName) {
                strncat(msg, "\n\t\tlabel: ", 1023);
                strncat(msg, callbackdata->pCmdBufLabels[i].pLabelName, 1023);
            }
        }
    }

    // append object list
    {
        strncat(msg, "\n\tamount of related objects: ", 1023);

        char amt[8];
        snprintf(amt, 8, "%d", callbackdata->objectCount);
        strncat(msg, amt, 1023);

        for (unsigned int i = 0; i < callbackdata->objectCount; i++) {
            if (callbackdata->pObjects[i].pObjectName) {
                strncat(msg, "\n\t\tlabel: ", 1023);
                strncat(msg, callbackdata->pObjects[i].pObjectName, 1023);
            }
        }
    }

    // since we aren't calling from _ori_ThrowError(), we need to check for errorCallback's validity first
    if (!_orion.callbacks.errorCallback) {
        _orion.callbacks.errorCallback = _ori_DefaultErrorCallback;
    }

    _orion.callbacks.errorCallback("VULKAN_DEBUG_MESSENGER", 0x03, msg, ORION_ERROR_SEVERITY_ERROR_BIT, userdata);

    return VK_FALSE;
}
