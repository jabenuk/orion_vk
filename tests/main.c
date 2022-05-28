#include <stdio.h>
#include <stdlib.h>
#include <glfw/include/GLFW/glfw3.h>

#include "orion.h"

#define WINDOW_NAME "Vulkan-Orion application"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

GLFWwindow *wMain = NULL;

oriState *state = NULL;
VkInstance instance = NULL;
    VkInstance temp;

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
    oriEnableDebugMessages(ORION_ERROR_SEVERITY_ALL_BIT);

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

    // specify layers to be enabled
    oriFlagLayerEnabled(state, "VK_LAYER_KHRONOS_validation");
}

void CreateInstance() {
    oriCreateStateVkInstance(state, &instance);
}

void Terminate() {
    oriFreeState(state);

    glfwTerminate();
}
