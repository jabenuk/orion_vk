/* *************************************************************************************** */
/*                       ORION GRAPHICS LIBRARY AND RENDERING ENGINE                       */
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


// ============================================================================ //
// *****                     Doxygen file information                     ***** //
// ============================================================================ //

/**
 * @file callback.c
 * @author jack bennett
 * @brief Default and internal callbacks
 *
 * @copyright Copyright (c) 2022 jack bennett
 *
 * This file contains definitions for callbacks used both internally within the
 * Orion library and as default options for user-set callbacks.
 *
 */

#include "orion.h"
#include "orion_funcs.h"

#include <string.h>
#include <stdio.h>


// ============================================================================ //
// *****                     Private/internal systems                     ***** //
// ============================================================================ //


// ----[Private/internal systems]---------------------------------------------- //
//                              Default callbacks                               //

void _oriDefaultDebugCallback(const char *name, const unsigned int code, const char *message, const oriSeverityBit_t severity, void *pointer) {
    // if the error message has been reported by a Vulkan debug messenger, we don't want to bother printing this other
    // stuff, as it is irrelevant.
    if (!strcmp(name, "VULKAN_DEBUG_MESSENGER")) {
        printf("(ori|vk!) %s\n", message);
        return;
    }

    char severitystr[15];

    switch (severity) {
        case ORION_DEBUG_SEVERITY_WARNING_BIT:
            strncpy(severitystr, "WARNING", 15);
            break;
        case ORION_DEBUG_SEVERITY_ERROR_BIT:
            strncpy(severitystr, "ERROR", 15);
            break;
        case ORION_DEBUG_SEVERITY_FATAL_BIT:
            strncpy(severitystr, "FATAL!", 15);
            break;
        default:
            // the severity will not be printed in logs and notifications.
            break;
    }

    // style messages differently if they aren't indicating any problems
    // (errors and fatal errors are normally standardised, with codes and such, but messages and warnings are not)
    switch (severity) {
        case ORION_DEBUG_SEVERITY_VERBOSE_BIT:
        case ORION_DEBUG_SEVERITY_NOTIF_BIT:
            printf("(ori!) %s\n", message);
            break;
        case ORION_DEBUG_SEVERITY_WARNING_BIT:
            printf("(ori!) WARNING: %s\n", message);
            break;
        default:
            printf("(ori!) %s: %s (code 0x%02X): \"%s\"\n", severitystr, name, code, message);
            break;
    }
}
