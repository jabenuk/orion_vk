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

/**
 * @file internal.h
 * @author jack bennett
 * @brief Internal shared header file
 *
 * @copyright Copyright (c) 2022
 *
 * This is an internal header.
 *
 * This file declares functions and macros, mostly helpers, that are widely used
 * within the internal scope of the Orion library - normally to reduce code reuse and
 * improve readability and therefore maintainability.
 *
 * This header is NOT to be included by the user, and is certainly not included as
 * part of the interface orion.h header.
 *
 */

#pragma once
#ifndef __ORION_INTERNAL_H
#define __ORION_INTERNAL_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "orion.h"
#include "structs.h"



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

// calls the global error callback in _orion with the given parameters.
void _ori_ThrowError(const char *name, unsigned int code, const char *message, oriErrorSeverityBit severity);

// standardised error kits:
// these should be used when calling the _ori_ThrowError() function.

#define ORERR_VULKAN_RETURN_ERROR \
    "ERR_VULKAN_RETURN_ERROR", \
    0x01, \
    "A Vulkan function returned a VkResult other than VK_SUCCESS.", \
    ORION_ERROR_SEVERITY_ERROR_BIT

#define ORERR_MEMORY_ERROR \
    "ERR_MEMORY_ERROR", \
    0x02, \
    "A function encountered a memory-related error.", \
    ORION_ERROR_SEVERITY_ERROR_BIT

// helper functions for specific 'error' types (improves on readability)
#define _ori_DebugLog(format, ...) \
    { \
        if (_orion.displayedErrorSeverities) { /* we should avoid string manipulation if it isn't necessary */ \
            char str[768]; \
            snprintf(str, 768, format, __VA_ARGS__); \
            _ori_ThrowError("", 0, str, ORION_ERROR_SEVERITY_VERBOSE_BIT); \
        } \
    }

#define _ori_Warning(format, ...) \
    { \
        if (_orion.displayedErrorSeverities) { /* we should avoid string manipulation if it isn't necessary */ \
            char str[768]; \
            snprintf(str, 768, format, __VA_ARGS__); \
            _ori_ThrowError("", 0, str, ORION_ERROR_SEVERITY_WARNING_BIT); \
        } \
    }

#define _ori_Notification(format, ...) \
    { \
        if (_orion.displayedErrorSeverities) { /* we should avoid string manipulation if it isn't necessary */ \
            char str[768]; \
            snprintf(str, 768, format, __VA_ARGS__); \
            _ori_ThrowError("", 0, str, ORION_ERROR_SEVERITY_NOTIF_BIT); \
        } \
    }

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_INTERNAL_H
