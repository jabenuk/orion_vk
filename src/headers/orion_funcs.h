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
 * @file orion_funcs.h
 * @author jack bennett
 * @brief Internal header file defining internal functions and callbacks.
 *
 * @copyright Copyright (c) 2022
 *
 * This is an internal header.
 * It is NOT to be included by the user, and is certainly not included as
 * part of the interface orion.h header.
 *
 * This file declares all internal functions - not specifically 'helper' functions,
 * as those are declared in orion_helpers.h.
 *
 * Callbacks are also declared here, and are all defined in lib/callback.c.
 *
 */

#pragma once
#ifndef __ORION_FUNCS_H
#define __ORION_FUNCS_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "orion.h"



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

// defined in lib/error.c
// calls the global error callback in _orion with the given parameters.
void _ori_ThrowError(const char *name, unsigned int code, const char *message, oriErrorSeverityBit severity);



// ============================================================================
// *****        DEFAULT CALLBACKS                                         *****
// ============================================================================

// defined in lib/callback.c
// default error callback (will be set in oriDefaultCallbacks())
void _ori_DefaultErrorCallback(
    const char *name,
    unsigned int code,
    const char *message,
    oriErrorSeverityBit severity,
    void *pointer
);

// defined in lib/callback.c
// this is always used for any debug messengers created under an Orion state
VKAPI_ATTR VkBool32 VKAPI_CALL _ori_VulkanDebugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagBitsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackdata,
    void *userdata
);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_FUNCS_H
