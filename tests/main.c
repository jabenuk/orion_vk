#include <stdio.h>
#include <stdlib.h>
#include <glfw/include/GLFW/glfw3.h>

#include "orion.h"

#define WINDOW_NAME "Vulkan-Orion application"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define DBGMSNGR_SEVERITIES VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
#define DBGMSNGR_TYPES VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT

GLFWwindow *wMain = NULL;

oriState *state = NULL;
VkInstance instance = NULL;
VkDebugUtilsMessengerEXT messenger = NULL;

void Initialise();      // library flags, initialisation
void CreateWindow();    // create window (wMain)
void CreateState();     // create state (state)
void CreateInstance();  // create instance (instance)
void Terminate();       // terminate the program

int main() {
    // initialisation and creation logic
    Initialise();
    CreateWindow();
    CreateState();
    CreateInstance();

    // terminate program
    Terminate();

    return 0;
}

void Initialise() {
    oriEnableLibDebugMessages(ORION_ERROR_SEVERITY_ALL_BIT);

    oriSetFlag(ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS, true);

    glfwInit();
}

void CreateWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    wMain = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
}

void CreateState() {
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
}

void CreateInstance() {
    oriCreateInstance(state, NULL, &instance);

    oriCreateDebugMessenger(state, &instance, NULL, &messenger, DBGMSNGR_SEVERITIES, DBGMSNGR_TYPES);
}

void Terminate() {
    oriFreeState(state);

    glfwTerminate();
}
