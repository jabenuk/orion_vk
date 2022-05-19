/* *************************************************************************************** */
/*                        ORION GRAPHICS LIBRARY AND RENDERING ENGINE                      */
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

#include "orion.h"
#include "internal.h"

#include <string.h>
#include <stdio.h>

// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================

// ============================================================================
// *****        DEFAULT CALLBACKS                                         *****
// ============================================================================

// default error callback (will be set in oriDefaultCallbacks())
void __DefaultErrorCallback(const char *name, unsigned int code, const char *message, oriErrorSeverity severity, void *pointer) {
    char severitystr[15];

    switch (severity) {
        case ORION_ERROR_SEVERITY_WARNING:
            strncpy(severitystr, "WARNING", 15);
            break;
        case ORION_ERROR_SEVERITY_ERROR:
            strncpy(severitystr, "ERROR", 15);
            break;
        case ORION_ERROR_SEVERITY_FATAL:
            strncpy(severitystr, "FATAL!", 15);
            break;
        default:
            // the severity will not be printed in logs and notifications.
            break;
    }

    // style messages differently if they aren't indicating any problems
    // (errors and fatal errors are normally standardised, with codes and such, but messages and warnings are not)
    switch (severity) {
        case ORION_ERROR_SEVERITY_VERBOSE:
        case ORION_ERROR_SEVERITY_NOTIF:
            printf("[orion] %s\n", message);
            break;
        case ORION_ERROR_SEVERITY_WARNING:
            printf("[orion] (%s) %s\n", severitystr, message);
            break;
        default:
            printf("[orion] (%s) %s (code 0x%02X): \"%s\"\n", severitystr, name, code, message);
            break;
    }
}

// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

void _ori_ThrowError(const char *name, unsigned int code, const char *message, oriErrorSeverity severity) {
    // don't bother with all this if the error callback's being suppressed anyways
    if (_orion.suppressErrorCallback) {
        return;
    }

    // also check if the error's severity is suppressed
    if ((_orion.suppressedErrorSeverities & severity) == severity) {
        return;
    }

    // set the default error callback if the current one is NULL
    if (!_orion.callbacks.errorCallback) {
        _orion.callbacks.errorCallback = __DefaultErrorCallback;
    }

    // throw error to callback
    _orion.callbacks.errorCallback(
        name,
        code,
        message,
        severity,
        _orion.callbacks.errorCallbackUserData
    );
}

// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================

// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

/**
 * @brief Set the global Orion error callback function.
 *
 * User data can be passed to the callback function (@ref oriErrorCallback) through the @c pointer
 * parameter. This will be recieved in the callback with the parameter of the same name.
 *
 * Pass NULL to the @c pointer parameter if you do not want to pass any data to the callback.
 *
 * @param callback the callback function to use.
 * @param pointer NULL or specified user data that will be sent to the callback.
 *
 * @sa oriErrorCallback
 *
 * @ingroup group_Errors
 *
 */
void oriSetErrorCallback(oriErrorCallback callback, void *pointer) {
    _orion.callbacks.errorCallback = callback;
    _orion.callbacks.errorCallbackUserData = pointer;
}

/**
 * @brief Suppress any debug messages that fall under the specified criteria.
 *
 * A list of error severities can be seen in the description of @ref oriErrorCallback.
 *
 * @param severities a bit field of severities to suppress
 *
 * @sa oriErrorCallback
 *
 * @ingroup group_Errors
 *
 */
void oriSuppressDebugMessages(oriErrorSeverity severities) {
    _orion.suppressedErrorSeverities |= severities;
}
