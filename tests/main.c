#include <stdio.h>

#include "orion.h"

int main() {
    // these are optional but here to demonstrate
    oriSetFlag(ORION_DISABLE_ERROR_CALLBACK, 0);
    oriSuppressDebugMessages(0);

    oriState *state = oriCreateState(VK_API_VERSION_1_3);
    oriSetStateApplicationInfo(state, NULL, "Orion-Vulkan application", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0));

    oriFreeState(state);

    return 0;
}
