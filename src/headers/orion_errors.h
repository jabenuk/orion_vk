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
 * @file orion_errors.h
 * @author jack bennett
 * @brief Internal header file declaring API error codes.
 *
 * @copyright Copyright (c) 2022 jack bennett
 *
 * This is an internal header.
 * It is NOT to be included by the user, and is certainly not included as
 * part of the interface orion.h header.
 *
 * This file declares standardised declarations of error codes commonly referenced
 * throughout the Orion API.
 *
 * The 'Error index' on the public documentation describes each error.
 *
 */

#pragma once
#ifndef __ORION_ERRORS_H
#define __ORION_ERRORS_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "orion.h"
#include "orion_structs.h"


// ============================================================================ //
// *****                     Private/internal systems                     ***** //
// ============================================================================ //


// ----[Private/internal systems]---------------------------------------------- //
//                           Error code declarations                            //

// Enum definitions based on their ID/code
//
typedef enum _oriErrorCode_t {
    // code 0xA1 is reserved for the recommended VULKAN_DEBUG_MESSENGER 'error'

    ORIERR_NULL_POINTER = 0x01,
    ORIERR_INSTANCE_CREATION_FAIL = 0x02,
    ORIERR_NOT_INIT = 0x03,
    ORIERR_INVALID_OBJECT = 0x04,
    ORIERR_VULKAN_QUERY_FAIL = 0x05,
    ORIERR_DEVICE_CREATION_FAIL = 0x06,

    ORIFERR_MEMORY_ERROR = 0xD0
} _oriErrorCode_t;


// ----[Private/internal systems]---------------------------------------------- //
//                      Error code-related helper function(s)                   //

// Helper function for use in internal error-throwing functions.
// 'extra' can be used to provide extra information to be concatenated onto the error description. NULL if n/a.
//
const _oriError_t _oriParseError(
    const _oriErrorCode_t id
);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_ERRORS_H
