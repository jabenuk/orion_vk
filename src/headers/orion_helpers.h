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

// append onto a dynamically allocated array
//      type: the type of values accepted by the array
//      array: the array, which will be updated
//      len: the array length variable, which will be updated
//      value: value to append
#define _ori_AppendOntoDArray(type, array, len, value) \
    { \
        type oldarr[len]; \
        /* copy current array onto stack */ \
        /* sidenote: using a common iterator like 'i' will render this macro useless in most for loops. */ \
        /*           so... we use a less likely one instead: */ \
        for (unsigned int UwU = 0; UwU < len; UwU++) { \
            oldarr[UwU] = (array)[UwU]; \
        } \
        \
        len++; \
        \
        array = realloc(array, len * sizeof(type)); \
        memset(array, 0, len * sizeof(type)); \
        if (!array) printf("Memory error -- calloc returned null!\n");\
        \
        /* copy old array contents into newly allocated array */ \
        for (unsigned int UwU = 0; UwU < len; UwU++) { \
            (array)[UwU] = oldarr[UwU]; \
        } \
        \
        /* append value onto the end of the array */ \
        (array)[len - 1] = value; \
    }

// remove the element at index from a dynamically allocated array
//      array: the array, which will be updated
//      len: the array length variable, which will be updated
//      index: the index of the element to remove
#define _ori_RemoveFromDArray(array, len, index) \
    { \
        if (index <= len) { \
            (array)[index] = 0; \
            \
            /* shift all later elements one to the left, filling the NULL 'gap' */ \
            for (unsigned int UwU = index; UwU < len - 1; UwU++) { \
                (array)[UwU] = (array)[UwU + 1]; \
            } \
            \
            len--; \
        } \
    }

// remove all duplicates from a dynamically allocated array
//      array: the array
//      len: length of the array
#define _ori_RemoveDArrayDuplicates(array, len) \
    { \
        for (unsigned int UwU = 0; UwU < len - 1; UwU++) { \
            if ((array)[UwU] != (array)[UwU + 1]) { \
                continue; \
            } \
            /* damn! we need to think of another unlikely name for an iterator. */ \
            /* that's easy: */ \
            for (unsigned int OwOWhatsThis = UwU + 1; OwOWhatsThis < len - 1; OwOWhatsThis++) { \
                (array)[OwOWhatsThis] = (array)[OwOWhatsThis + 1]; \
            } \
            /* annoyingly long but... it is a bit funny. */ \
            len--; \
            UwU--; \
        } \
    }

// free a dynamically allocated array
//      array: the array
//      len: length of the array
#define _ori_FreeDArray(array, len) \
    { \
        free(array); \
        array = NULL; \
        len = 0; \
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
