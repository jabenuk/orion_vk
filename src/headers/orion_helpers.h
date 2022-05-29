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
 * @file orion_helpers.h
 * @author jack bennett
 * @brief Internal header file defining internal helper functions.
 *
 * @copyright Copyright (c) 2022
 *
 * This is an internal header.
 * It is NOT to be included by the user, and is certainly not included as
 * part of the interface orion.h header.
 *
 * This file declares functions and macros that are widely used
 * within the internal scope of the Orion library - normally to reduce code reuse and
 * improve readability and therefore maintainability.
 *
 */

#pragma once
#ifndef __ORION_HELPERS_H
#define __ORION_HELPERS_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        GENERAL HELPER FUNCTIONS                                  *****
// ============================================================================

// helper macro to remove duplicates from a dynamically allocated array
#define _ori_RemoveArrayDuplicates(array, count) \
    { \
        for (unsigned int i = 0; i < count - 1; i++) { \
            if (array[i] != array[i + 1]) { \
                continue; \
            } \
            for (unsigned int j = i + 1; j < count - 1; j++) { \
                array[j] = array[j + 1]; \
            } \
            count--; \
            i--; \
        } \
    }



// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

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

#endif // __ORION_HELPERS_H
