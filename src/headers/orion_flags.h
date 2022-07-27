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
#ifndef __ORION_FLAGS_H
#define __ORION_FLAGS_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus


// ============================================================================ //
// *****                     Private/internal systems                     ***** //
// ============================================================================ //


// ----[Private/internal systems]---------------------------------------------- //
//                            Flag/macro definitions                            //

// Maximum amount of characters in debug log messages sent by functions like oriInit().
//
#define MAX_LOG_LEN 768

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_FLAGS_H
