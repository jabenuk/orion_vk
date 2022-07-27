#include "orion.h"

#include "glfw/include/GLFW/glfw3.h"

#include <stdio.h>
#include <string.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_NAME "orion_vk window"

GLFWwindow *window_Main;
VkSurfaceKHR surface_Main;

VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;
VkDevice device;

// callback for debug messengers
//
VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagBitsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackdata,
    void *userdata
);

int main() {
    // ===========================================
    // initialise program
    //

    // initialise GLFW
    if (!glfwInit()) {
        printf("failed to init glfw\n");
        return -1;
    }

    // setup debugging
    oriConfigureDebugMessages(ORION_DEBUG_SEVERITY_ALL_BIT);

    // ===========================================
    // create Vulkan instance
    //

    // list of layers to enable
    const char *layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    // list of instance extensions to enable
    const char *instanceExtensions[] = {
        "VK_KHR_surface",
        "VK_KHR_xcb_surface",
        "VK_EXT_debug_utils"
    };

    // create debug messenger
    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pNext = NULL,
        .pUserData = NULL,
        .pfnUserCallback = vulkanCallback
    };

    // create instance + initialise everything
    oriInit(1, &instance, 0, VK_API_VERSION_1_3, "Orion application", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), 1, layers, 3, instanceExtensions, &debugUtilsMessengerInfo);

    // load necessary non-core Vulkan functions for the program
    PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    // ===========================================
    // create debug messenger
    //

    // create a debug messenger with the same create-info that was passed to oriInit for the instance's pNext
    if (CreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerInfo, NULL, &debugMessenger)) {
        printf("failed to create debug messenger\n");
        return -1;
    }

    // ===========================================
    // create interface
    //

    // create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window_Main = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
    if (!window_Main) {
        printf("failed to create window\n");
        return -1;
    }

    // create surface
    if (glfwCreateWindowSurface(instance, window_Main, NULL, &surface_Main)) {
        printf("failed to create surface\n");
        return -1;
    }

    // ===========================================
    // create logical device with first available physical device
    //

    // get available physical devices
    VkPhysicalDevice *suitablePhysicalDevices;
    unsigned int suitablePhysicalDeviceCount;
    oriEnumerateSuitablePhysicalDevices(&instance, NULL, &suitablePhysicalDeviceCount, &suitablePhysicalDevices);

    // get available queue families
    VkQueueFamilyProperties *queueFamilyProperties;
    unsigned int queueFamilyPropertyCount;
    oriEnumerateAvailableQueueFamilies(&suitablePhysicalDevices[0], &queueFamilyPropertyCount, &queueFamilyProperties);

    // get necessary queue family indices
    struct {
        unsigned int graphics;
        unsigned int present;
    } queueFamilyIndices = { 0 };
    const unsigned int queueFamilyIndexCount = 2;

    for (unsigned int i = 0; i < queueFamilyPropertyCount; i++) {
        // check for graphics queue
        if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyIndices.graphics = i;
        }

        // check for surface support; if the queue family has surface support, it is a present queue
        VkBool32 psupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(suitablePhysicalDevices[0], i, surface_Main, &psupport);

        if (psupport) {
            queueFamilyIndices.present = i;
        }
    }

    // create an initinfo struct for each queue (one from each queue family)
    VkDeviceQueueCreateInfo queueCreateInfos[queueFamilyIndexCount];

    {
        float queuePriority = 1.0f;

        // array of unique queue indices
        unsigned int uniqueQueueIndices[] = {
            queueFamilyIndices.graphics,
            queueFamilyIndices.present
        };

        for (unsigned int i = 0; i < queueFamilyIndexCount; i++) {
            queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[i].pNext = NULL;
            queueCreateInfos[i].flags = 0;
            queueCreateInfos[i].queueFamilyIndex = uniqueQueueIndices[i];
            queueCreateInfos[i].queueCount = 1;
            queueCreateInfos[i].pQueuePriorities = &queuePriority;
        }
    }

    // list of device extensions to enable
    const char *deviceExtensions[] = {
        "VK_KHR_swapchain"
    };

    // create logical device
    oriCreateLogicalDevice(&device, 0, suitablePhysicalDevices[0], queueFamilyIndexCount, queueCreateInfos, 1, deviceExtensions, NULL, NULL);

    // ===========================================
    // main loop
    //

    // window loop
    // while (!glfwWindowShouldClose(window_Main)) {
    //     glfwSwapBuffers(window_Main);
    //     glfwPollEvents();
    // }

    // ===========================================
    // termination of API
    //

    // destroy Vulkan objects BEFORE oriTerminate(), as that function will destroy the instance
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, oriGetVulkanAllocators());
    vkDestroySurfaceKHR(instance, surface_Main, oriGetVulkanAllocators());
    vkDestroyDevice(device, oriGetVulkanAllocators());

    // terminate library and destroy the instance created with oriInit().
    oriTerminate(true);

    return 0;
}

// callback for debug messengers
//
VKAPI_ATTR VkBool32 VKAPI_CALL vulkanCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagBitsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *callbackdata, void *userdata) {
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
