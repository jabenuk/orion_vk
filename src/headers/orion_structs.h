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
 * @file orion_structs.h
 * @author jack bennett
 * @brief Internal header file defining public opaque structures.
 *
 * @copyright Copyright (c) 2022
 *
 * This is an internal header.
 * It is NOT to be included by the user, and is certainly not included as
 * part of the interface orion.h header.
 *
 * This file defines the members of public, but opaque, library structures.
 *
 * As these structures are widely referred to throughout the library, their definitions have
 * been merged into this one header file as opposed to, for example, defining a 'buffer' struct
 * in some sort of 'buffer.c' source file (in that case, it could only be directly modified in
 * said 'buffer.c' file).
 *
 */

#pragma once
#ifndef __ORION_STRUCTS_H
#define __ORION_STRUCTS_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "orion.h"



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================

typedef struct _ori_Lib {
    struct {
        oriErrorCallback errorCallback;
        void *errorCallbackUserData;
    } callbacks;

    oriErrorSeverityBit displayedErrorSeverities;

    struct {
        bool createInstanceDebugMessengers;
    } flags;
} _ori_Lib;
extern _ori_Lib _orion;



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC (OPAQUE) STRUCTURES                          *****
// ----------------------------------------------------------------------------
// ============================================================================

/**
 * @brief An opaque structure that holds all public state.
 *
 * This structure is mostly used to store lists of created Orion objects so they can be implicitly destroyed in oriTerminate().
 *
 * However, it also holds state related to Vulkan, such as the application info.
 *
 * @sa oriCreateState()
 * @sa oriFreeState()
 *
 * @ingroup group_Meta
 *
 */
typedef struct oriState {
    // struct of all global dynamic arrays (use realloc() to extend)
    // these must be used instead of linked lists when they are of non-Orion structs (that don't have a 'next' pointer member)
    struct {
        VkInstance **instances;                     unsigned int instancesCount;
        VkDebugUtilsMessengerEXT **debugMessengers; unsigned int debugMessengersCount;
    } arrays;

    VkApplicationInfo appInfo;

    struct {
        // enabled Vulkan layers for instances created under this state
        char **enabledLayers;       unsigned int enabledLayerCount;

        // enabled Vulkan instance extensions for instances created under this state
        char **enabledExtensions;   unsigned int enabledExtCount;

        struct {
            VkDebugUtilsMessageSeverityFlagsEXT severities;
            VkDebugUtilsMessageTypeFlagsEXT types;
        } dbgmsngrEnabledMessages;
    } instanceCreateInfo;
} oriState;

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_STRUCTS_H
