#include <stdio.h>
#include <stdlib.h>

#include "orion.h"
#include <glfw/include/GLFW/glfw3.h>

#define WINDOW_NAME "Vulkan-Orion application"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define DBGMSNGR_SEVERITIES VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
#define DBGMSNGR_TYPES VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT

GLFWwindow *wMain = NULL;
VkSurfaceKHR sMain = NULL;

oriState *state = NULL;

VkInstance instance = NULL;
VkDebugUtilsMessengerEXT messenger = NULL;
VkDevice logicalDevice = NULL;

bool physicalDeviceSuitabilityCheckFunc(VkPhysicalDevice device);

int main() {
    //
    // initialise program
    //

    oriEnableLibDebugMessages(ORION_ERROR_SEVERITY_ALL_BIT);

    oriSetFlag(ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS, true);

    glfwInit();

    //
    // create GLFW window
    //

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    wMain = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);

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

    oriCreateDebugMessenger(state, &instance, NULL, DBGMSNGR_SEVERITIES, DBGMSNGR_TYPES, &messenger);

    //
    // create a surface with GLFW
    //

    if (glfwCreateWindowSurface(instance, wMain, NULL, &sMain)) {
        printf("FATAL!! Failed to create window surface!\n");
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
    oriEnumerateAvailableQueueFamilies(devices[0], NULL, NULL);

    // create logical device
    // oriCreateLogicalDevice(state, 1, &(devices[0]), NULL, familyCount, families, 0, NULL, NULL, &logicalDevice);

    //
    // terminate
    //

    free(devices);
    devices = NULL;

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
