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

// this is mostly used to store lists of enabled Vulkan layers, features, and extensions
typedef struct _ori_StrList {
    struct _ori_StrList *next;
    char *data;
} _ori_StrList;

typedef struct _ori_LibState {
    struct {
        oriErrorCallback errorCallback;
        void *errorCallbackUserData;
    } callbacks;

    oriErrorSeverityBit displayedErrorSeverities;
} _ori_LibState;
extern _ori_LibState _orion;

typedef struct _ori_LibFlags {
} _ori_LibFlags;
extern _ori_LibFlags _orionflags;



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
    // struct of all global linked lists
    struct {
    } listHeads;

    VkApplicationInfo appInfo; // this cannot (?) be freed after VkCreateInstance() so it isn't in the instanceCreateInfo struct.
    unsigned int apiVersion;

    // pointer to the associated instance structure
    VkInstance *instance;

    // this struct is freed after oriCreateStateVkInstance().
    struct {
        // enabled Vulkan layers
        unsigned int enabledLayerCount;
        _ori_StrList *enabledLayerListHead;

        // enabled Vulkan extensions
        unsigned int enabledExtCount;
        _ori_StrList *enabledExtListHead;
    } instanceCreateInfo;
} oriState;

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_STRUCTS_H
