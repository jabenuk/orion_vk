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

#pragma once
#ifndef __ORION_INTERNAL_H
#define __ORION_INTERNAL_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "orion.h"
#include <stdbool.h>



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        STATIC INTERNAL STATE                                     *****
// ============================================================================

typedef struct _ori_LibState {
    struct {
        oriErrorCallback errorCallback;
        void *errorCallbackUserData;
    } callbacks;

    oriErrorSeverity displayedErrorSeverities;
} _ori_LibState;

extern _ori_LibState _orion;



// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

// calls the global error callback in _orion with the given parameters.
void _ori_ThrowError(const char *name, unsigned int code, const char *message, oriErrorSeverity severity);

// standardised error kits:
// these should be used when calling the _ori_ThrowError() function.

#define ORERR_INVALID_LIB_FLAG \
    "ERR_INVALID_LIB_FLAG", \
    0x01, \
    "An invalid flag was given to oriSetFlag(); nothing was updated.", \
    ORION_ERROR_SEVERITY_WARNING

// helper functions for specific 'error' types (improves on readability)

#define _ori_DebugLog(format, ...) \
    { \
        if (_orion.displayedErrorSeverities) { /* we should avoid string manipulation if it isn't necessary */ \
            char str[256]; \
            snprintf(str, 256, format, __VA_ARGS__); \
            _ori_ThrowError("", 0, str, ORION_ERROR_SEVERITY_VERBOSE); \
        } \
    }

#define _ori_Warning(format, ...) \
    { \
        if (_orion.displayedErrorSeverities) { /* we should avoid string manipulation if it isn't necessary */ \
            char str[256]; \
            snprintf(str, 256, format, __VA_ARGS__); \
            _ori_ThrowError("", 0, str, ORION_ERROR_SEVERITY_WARNING); \
        } \
    }

#define _ori_Notification(format, ...) \
    { \
        if (_orion.displayedErrorSeverities) { /* we should avoid string manipulation if it isn't necessary */ \
            char str[256]; \
            snprintf(str, 256, format, __VA_ARGS__); \
            _ori_ThrowError("", 0, str, ORION_ERROR_SEVERITY_NOTIF); \
        } \
    }

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_INTERNAL_H
