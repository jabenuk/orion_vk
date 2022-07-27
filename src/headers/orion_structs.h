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
 * @file orion_structs.h
 * @author jack bennett
 * @brief Internal header file defining public opaque structures.
 *
 * @copyright Copyright (c) 2022 jack bennett
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

#include "uthash/include/uthash.h"


// ============================================================================ //
// *****                     Private/internal systems                     ***** //
// ============================================================================ //


// ----[Private/internal systems]---------------------------------------------- //
//                        Internal structure definitions                        //

typedef struct _oriLibrary_t _oriLibrary_t;
typedef struct _oriError_t _oriError_t;

typedef struct _oriVkInstance_t _oriVkInstance_t;
typedef struct _oriVkDevice_t _oriVkDevice_t;

// Struct to hold global library data
//
typedef struct _oriLibrary_t {
    bool initialised;

    oriSeverityBit_t debugMessageSeverities;

    struct {
        struct {
            oriDebugCallbackfun fun;
            void *pointer;
        } debug;

        VkAllocationCallbacks *vulkanAllocators;
    } callbacks;

    // struct of hashtables of pointers to Orion-created Vulkan structures
    struct {
        _oriVkInstance_t *vkInstances;
        _oriVkDevice_t *vkDevices;
    } allocatees;
} _oriLibrary_t;

// Global state
//
extern _oriLibrary_t _orion;

// Struct to hold information about a standardised error for use in internal error-handling functions
//
typedef struct _oriError_t {
    const char *name;
    const char *description;
} _oriError_t;

// Hashable Vulkan wrapper struct to hold extra data about an instance
// There should only ever be one instance anyway, but we are doing it this way for consistency between instances and other Vulkan structures.
// There could also be an update to Vulkan in the future which makes it more useful to have multiple instances, in which
// case having this design makes it easier to adapt to that.
//
typedef struct _oriVkInstance_t {
    UT_hash_handle hh;

    VkInstance *handle; // serves as both the location of the instance handle and the hash key

    char **layers;
    unsigned int layerCount;
    char **extensions;
    unsigned int extensionCount;
} _oriVkInstance_t;

// Hashable Vulkan logical device wrapper struct
//
typedef struct _oriVkDevice_t {
    UT_hash_handle hh;

    VkDevice *handle;

    char **extensions;
    unsigned int extensionCount;
} _oriVkDevice_t;


#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_STRUCTS_H
