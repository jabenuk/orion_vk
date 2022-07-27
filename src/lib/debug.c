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
 * @file debug.c
 * @author jack bennett
 * @brief Default and internal error-related functions
 *
 * @copyright Copyright (c) 2022 jack bennett
 *
 * This file contains definitions for internally-used helper functions related
 * to library error handling and throwing, as well as public API debugging
 * measures.
 *
 */

#include "orion.h"
#include "orion_funcs.h"
#include "orion_errors.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


// ============================================================================ //
// *****                     Private/internal systems                     ***** //
// ============================================================================ //


// ----[Private/internal systems]---------------------------------------------- //
//                      Error code-related helper function(s)                   //

// The error enums are defined in headers/orion_errors.h.
const _oriError_t _oriParseError(const _oriErrorCode_t id) {
    switch (id) {
        default:
            return (_oriError_t) { NULL };

        case ORIERR_NULL_POINTER:
            return (_oriError_t) {
                .name = "ERR_NULL_POINTER",
                .description = "function recieved NULL pointer instead of required arg"
            };
        case ORIERR_INSTANCE_CREATION_FAIL:
            return (_oriError_t) {
                .name = "ERR_INSTANCE_CREATION_FAIL",
                .description = "Vulkan failed to create instance"
            };
        case ORIERR_NOT_INIT:
            return (_oriError_t) {
                .name = "ERR_NOT_INIT",
                .description = "call oriInit() once before this function"
            };
        case ORIERR_INVALID_OBJECT:
            return (_oriError_t) {
                .name = "ERR_INVALID_OBJECT",
                .description = "invalid Vulkan object or was not created with Orion"
            };
        case ORIERR_VULKAN_QUERY_FAIL:
            return (_oriError_t) {
                .name = "ERR_VULKAN_QUERY_FAIL",
                .description = "Vulkan query function failed"
            };
        case ORIERR_DEVICE_CREATION_FAIL:
            return (_oriError_t) {
                .name = "ERR_DEVICE_CREATION_FAIL",
                .description = "Vulkan failed to create logical device"
            };

        case ORIFERR_MEMORY_ERROR:
            return (_oriError_t) {
                .name = "FERR_MEMORY_ERROR",
                .description = "native memory error"
            };
    }
}


// ----[Private/internal systems]---------------------------------------------- //
//                        Error-throwing helper functions                       //

#define MAX_ERRORMSG_LEN 512

// macro here which will be used in _ori(Log|Notification|Warning) to reduce code reuse.
//
#define DBGCB_CALL_NOTERR(sev) \
{ \
    va_list varargs; \
    va_start(varargs, format); \
 \
    /* format message string with given format + variadic arguments */ \
    char msg[MAX_ERRORMSG_LEN]; \
    vsnprintf(msg, MAX_ERRORMSG_LEN, format, varargs); \
 \
    va_end(varargs); \
 \
    _orion.callbacks.debug.fun("", 0x0, msg, sev, _orion.callbacks.debug.pointer); \
}

// another macro, same reason as above but for _ori(Error|FatalError).
//
#define DBGCB_CALL_ERR(sev) \
{ \
    _oriError_t err = _oriParseError(id); \
 \
    /* format message with extra information.
       the extra information will be surrounded by brackets if it was given. */ \
    char msg[MAX_ERRORMSG_LEN]; \
    snprintf(msg, MAX_ERRORMSG_LEN, "%s%s%s%s", err.description, (extra) ? " (" : "", (extra) ? extra : "", (extra) ? ")" : ""); \
 \
    _orion.callbacks.debug.fun(err.name, id, msg, sev, _orion.callbacks.debug.pointer); \
}

// another macro to check if the specified severity is allowed to be displayed
// if sev is not to be shown, then the parent function returns
//
#define DBGCB_VERIFY_SEV(sev) \
{ \
    if ((_orion.debugMessageSeverities & sev) != sev) { \
        return; \
    } \
}

void _oriLog(const char *format, ...) {
    DBGCB_VERIFY_SEV(ORION_DEBUG_SEVERITY_VERBOSE_BIT);
    DBGCB_CALL_NOTERR(ORION_DEBUG_SEVERITY_VERBOSE_BIT);
}

void _oriNotification(const char *format, ...) {
    DBGCB_VERIFY_SEV(ORION_DEBUG_SEVERITY_NOTIF_BIT);
    DBGCB_CALL_NOTERR(ORION_DEBUG_SEVERITY_NOTIF_BIT);
}

void _oriWarning(const char *format, ...) {
    DBGCB_VERIFY_SEV(ORION_DEBUG_SEVERITY_WARNING_BIT);
    DBGCB_CALL_NOTERR(ORION_DEBUG_SEVERITY_WARNING_BIT);
}

void _oriError(const _oriErrorCode_t id, const char *extra) {
    // errors will always be shown regardless of debug message configuration
    DBGCB_CALL_ERR(ORION_DEBUG_SEVERITY_ERROR_BIT);
}
#
void _oriFatalError(const _oriErrorCode_t id, const char *extra) {
    // fatal errors will always be shown regardless of debug message configuration
    DBGCB_CALL_ERR(ORION_DEBUG_SEVERITY_FATAL_BIT);

    // terminate the program
    // NOTE that this does not properly terminate the library or free memory!
    printf("HALT_AND_CATCH_FIRE\n");
    exit(EXIT_FAILURE);
}

// ============================================================================ //
// *****                  Orion library public interface                  ***** //
// ============================================================================ //


// ----[Orion library public interface]---------------------------------------- //
//                                  Debugging                                   //

const oriReturnStatus_t oriConfigureDebugMessages(const oriSeverityBit_t severities) {
#   ifdef __oridebug
        _oriLog("debug message configuration updated (severities: bit field 0x%02X) (%s)", severities, __func__);
#   endif

    _orion.debugMessageSeverities = severities;
    return ORION_RETURN_STATUS_OK;
}

const oriReturnStatus_t oriSetDebugCallback(const oriDebugCallbackfun callback, void *pointer) {
#   ifdef __oridebug
        _oriLog("debug callback updated (user data %p) (%s)", pointer, __func__);
#   endif

    // if a callback was specified then use it
    if (callback) {
        _orion.callbacks.debug.fun = callback;
    } else {
        // otherwise use the default
        _orion.callbacks.debug.fun = _oriDefaultDebugCallback;
    }

    // update the user data pointer store
    _orion.callbacks.debug.pointer = pointer;

    return ORION_RETURN_STATUS_OK;
}

const oriDebugCallbackfun oriGetDebugCallback() {
    return _orion.callbacks.debug.fun;
}

const void *oriGetDebugCallbackUserData() {
    return _orion.callbacks.debug.pointer;
}

const char *oriStringifyReturnStatus(const oriReturnStatus_t status) {
    switch (status) {
        case ORION_RETURN_STATUS_OK:            return "function executed successfully (OK)";
        case ORION_RETURN_STATUS_SKIPPED:       return "function skipped (SKIPPED)";
        case ORION_RETURN_STATUS_NO_OUTPUT:     return "function recieved NULL output pointers, returned nothing (NO_OUTPUT)";
        case ORION_RETURN_STATUS_NULL_POINTER:  return "function recieved NULL in place of a required parameter(s) (NULL_POINTER)";
        case ORION_RETURN_STATUS_ERROR:         return "error encountered (ERROR)";
    }

    // invalid status enum
    return "unknown";
}
