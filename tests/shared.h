#ifndef __SHARED_H
#define __SHARED_H

#include "orion.h"
#include "glfw/include/GLFW/glfw3.h"

#include <stdio.h>
#include <string.h>

// callback for debug messengers
//
VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallback(
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
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT ||
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
    ) {
        snprintf(msg, 1024, "%s >> %s", severityStr, callbackdata->pMessage);

        // call the orion callback
        oriGetDebugCallback()("VULKAN_DEBUG_MESSENGER", 0x03, msg, ORION_DEBUG_SEVERITY_ERROR_BIT, userdata);
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

    // call the orion callback
    oriGetDebugCallback()("VULKAN_DEBUG_MESSENGER", 0x03, msg, ORION_DEBUG_SEVERITY_ERROR_BIT, userdata);

    return VK_FALSE;
}

#endif // __SHARED_H
