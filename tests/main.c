#include <stdio.h>

#include "orion.h"

int main() {
    oriEnableDebugMessages(ORION_ERROR_SEVERITY_MAX_BIT);

    oriState *state = oriCreateState(VK_API_VERSION_1_3);
    oriSetStateApplicationInfo(state, NULL, "Orion-Vulkan application", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0));

    oriFreeState(state);

    return 0;
}
