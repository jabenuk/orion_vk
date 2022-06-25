#include <stdio.h>
#include <stdlib.h>

#include "orion.h"
#include <glfw/include/GLFW/glfw3.h>

#define WINDOW_NAME "Vulkan-Orion application"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// #define ORION_SEVERITIES ORION_ERROR_SEVERITY_FATAL_BIT | ORION_ERROR_SEVERITY_ERROR_BIT | ORION_ERROR_SEVERITY_WARNING_BIT
#define ORION_SEVERITIES ORION_ERROR_SEVERITY_ALL_BIT

#define DBGMSNGR_SEVERITIES VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
#define DBGMSNGR_TYPES VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT

GLFWwindow *wMain = NULL;
VkSurfaceKHR sMain = NULL;

oriState *state = NULL;

VkInstance instance = NULL;
VkDebugUtilsMessengerEXT messenger = NULL;

VkDevice logicalDevice = NULL;
VkQueue graphicsQueue = NULL;
VkQueue presentQueue = NULL;

bool physicalDeviceSuitabilityCheckFunc(VkPhysicalDevice device);

int main() {
    //
    // initialise program
    //

    oriEnableLibDebugMessages(ORION_SEVERITIES);

    oriSetFlag(ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS, true);

    if (!glfwInit()) {
        printf("FATAL!! Failed to init GLFW!\n");
        return -1;
    }

    //
    // create Orion state
    //

    state = oriCreateState();
    oriDefineStateApplicationInfo(state, NULL, VK_API_VERSION_1_3, WINDOW_NAME, VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0));

    // get GLFW required extensions
    const char **extensions;

    unsigned int extensionCount;
    extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    // specify GLFW required extensions to be enabled
    for (unsigned int i = 0; i < extensionCount; i++) {
        oriFlagInstanceExtensionEnabled(state, extensions[i]);
    }

    // specify debug utils extension
    oriFlagInstanceExtensionEnabled(state, "VK_EXT_debug_utils");

    // specify layers to be enabled
    oriFlagLayerEnabled(state, "VK_LAYER_KHRONOS_validation");

    // specify debug suppressions
    oriSpecifyInstanceDebugMessages(state, DBGMSNGR_SEVERITIES, DBGMSNGR_TYPES);

    //
    // create a Vulkan instance
    //

    oriCreateInstance(state, NULL, &instance);

    // also create a debug messenger
    oriCreateDebugMessenger(state, &instance, NULL, DBGMSNGR_SEVERITIES, DBGMSNGR_TYPES, &messenger);

    //
    // create GLFW window
    //

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    wMain = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
    if (!wMain) {
        printf("FATAL!! Failed to create window!\n");
        return -1;
    }

    if (glfwCreateWindowSurface(instance, wMain, NULL, &sMain)) {
        printf("FATAL!! Failed to create surface!\n");
        return -1;
    }


    //
    // get an array of all physical devices and create a logical device with the first one
    //

    // enumerate suitable devices
    VkPhysicalDevice *devices;
    unsigned int devicecount;
    oriEnumerateSuitablePhysicalDevices(instance, physicalDeviceSuitabilityCheckFunc, &devicecount, &devices);

    // enumerate available queue families
    VkQueueFamilyProperties *families;
    unsigned int familyCount;
    oriEnumerateAvailableQueueFamilies(&devices[0], &familyCount, &families);

    // get graphics and present queue family indices
    struct {
        unsigned int graphics;
        unsigned int present;
    } queueFamilyIndices = { 0 };
    const unsigned int uniqueQueueCount = 2;

    {
        for (unsigned int i = 0; i < familyCount; i++) {
            // check for graphics queue
            if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyIndices.graphics = i;
            }

            // check for surface support; if the queue family has surface support, it is a present queue
            VkBool32 psupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(devices[0], i, sMain, &psupport);

            if (psupport) {
                queueFamilyIndices.present = i;
            }
        }
    }

    // create a creation info struct for each queue family
    VkDeviceQueueCreateInfo queueCreateInfos[uniqueQueueCount];

    {
        float queuePriority = 1.0f;

        // array of unique queue indices
        unsigned int uniqueQueueIndices[] = {
            queueFamilyIndices.graphics,
            queueFamilyIndices.present
        };

        for (unsigned int i = 0; i < uniqueQueueCount; i++) {
            queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[i].pNext = NULL;
            queueCreateInfos[i].flags = 0;
            queueCreateInfos[i].queueFamilyIndex = uniqueQueueIndices[i];
            queueCreateInfos[i].queueCount = 1;
            queueCreateInfos[i].pQueuePriorities = &queuePriority;
        }
    }

    // create logical device
    const char *deviceExtensions[] = {
        "VK_KHR_swapchain"
    };

    oriCreateLogicalDevice(state, 1, &(devices[0]), NULL, uniqueQueueCount, queueCreateInfos, 1, deviceExtensions, NULL, &logicalDevice);

    // get queue handles
    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphics, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.present, 0, &presentQueue);

    //
    // terminate
    //

    free(devices); devices = NULL;
    free(families); families = NULL;

    // because the window surface wasn't created with an Orion function, we need to manually destroy it
    vkDestroySurfaceKHR(instance, sMain, NULL);

    oriDestroyState(state);

    glfwTerminate();

    return 0;
}

bool physicalDeviceSuitabilityCheckFunc(VkPhysicalDevice device) {
    bool r = false;

    return true;
}
