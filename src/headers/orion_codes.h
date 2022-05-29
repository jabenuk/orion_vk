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
 * @file orion_codes.h
 * @author jack bennett
 * @brief Internal header file defining error codes, possibly amongst other things in the future.
 *
 * @copyright Copyright (c) 2022
 *
 * This is an internal header.
 * It is NOT to be included by the user, and is certainly not included as
 * part of the interface orion.h header.
 *
 * This file declares standardised macros for things commonly referenced in the
 * Orion library - mostly errors.
 *
 */

#pragma once
#ifndef __ORION_CODES_H
#define __ORION_CODES_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

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

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_CODES_H
