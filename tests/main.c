#include <stdio.h>
#include <stdlib.h>
#include <glfw/include/GLFW/glfw3.h>

#include "orion.h"

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Vulkan-Orion application", NULL, NULL);

    oriEnableDebugMessages(ORION_ERROR_SEVERITY_ALL_BIT);

    oriState *state = oriCreateState(VK_API_VERSION_1_3);

    oriDefineStateApplicationInfo(state, NULL, "Orion-Vulkan application", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0));

    oriFlagLayerEnabled(state, "VK_LAYER_KHRONOS_validation");

    const char **extensions;
    unsigned int extensionCount;
    extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    for (unsigned int i = 0; i < extensionCount; i++) {
        oriFlagInstanceExtensionEnabled(state, extensions[i]);
    }

    VkInstance instance;
    oriCreateStateVkInstance(state, &instance);

    oriFreeState(state);

    glfwTerminate();
    return 0;
}
