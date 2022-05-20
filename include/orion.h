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

/*********************************************************/
/*                                                       */
/*                          88                           */
/*                          ""                           */
/*                                                       */
/*    ,adPPYba,  8b,dPPYba, 88  ,adPPYba,  8b,dPPYba,    */
/*   a8"     "8a 88P'   "Y8 88 a8"     "8a 88P'   `"8a   */
/*   8b       d8 88         88 8b       d8 88       88   */
/*   "8a,   ,a8" 88         88 "8a,   ,a8" 88       88   */
/*    `"YbbdP"'  88         88  `"YbbdP"'  88       88   */
/*                                                       */
/*                                                       */
/*                                                       */
/*********************************************************/

/**
 * @file orion.h
 * @brief The public header of the core Orion library.
 *
 * @details
 * This is the public header for including Orion.
 * All core public library functionality is declared here.
 *
 */

#pragma once
#ifndef __ORION_H
#define __ORION_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include <vulkan/vulkan.h>


// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

// error severities (bit field):
typedef enum oriErrorSeverity {
    ORION_ERROR_SEVERITY_MAX_BIT =  0xFF,   // 0b11111111
    ORION_ERROR_SEVERITY_FATAL =    0x01,   // 0b00000001
    ORION_ERROR_SEVERITY_ERROR =    0x02,   // 0b00000010
    ORION_ERROR_SEVERITY_WARNING =  0x04,   // 0b00000100
    ORION_ERROR_SEVERITY_NOTIF =    0x08,   // 0b00001000
    ORION_ERROR_SEVERITY_VERBOSE =  0x10    // 0b00010000
} oriErrorSeverity;

/**
 * @brief Callback function for general runtime errors
 *
 * This function signature must be followed when creating an error callback for debugging.
 * The global error callback is set in oriSetErrorCallback().
 *
 * | List of available error severities | Description                                                           |
 * | ---------------------------------- | --------------------------------------------------------------------- |
 * | @c ORION_ERROR_SEVERITY_FATAL      | errors that the program @b cannot @b recover @b from.                 |
 * | @c ORION_ERROR_SEVERITY_ERROR      | significant but recoverable errors.                                   |
 * | @c ORION_ERROR_SEVERITY_WARNING    | events that may cause problems, but are not directly too significant. |
 * | @c ORION_ERROR_SEVERITY_NOTIF      | general events, no problems reported.                                 |
 * | @c ORION_ERROR_SEVERITY_VERBOSE    | @b every event that is happening.                                     |
 *
 * @param name the name of the error ID
 * @param code the error ID / code
 * @param message a message for debugging
 * @param severity the severity of the error
 * @param pointer NULL or a user-specified pointer (can be defined in oriSetErrorCallback())
 *
 * @sa oriSetErrorCallback()
 *
 * @ingroup group_Errors
 *
 */
typedef void (* oriErrorCallback)(const char *name, unsigned int code, const char *message, oriErrorSeverity severity, void *pointer);

/**
 * @brief Set the global Orion error callback function.
 *
 * User data can be passed to the callback function (@ref oriErrorCallback) through the @c pointer
 * parameter. This will be recieved in the callback with the parameter of the same name.
 *
 * Pass NULL to the @c callback parameter to use the default, built-in error callback.
 *
 * Pass NULL to the @c pointer parameter if you do not want to pass any data to the callback.
 *
 * @param callback the callback function to use.
 * @param pointer NULL or specified user data that will be sent to the callback.
 *
 * @sa oriErrorCallback
 *
 * @ingroup group_Errors
 *
 */
void oriSetErrorCallback(oriErrorCallback callback, void *pointer);

/**
 * @brief Recieve any debug messages that fall under the specified criteria.
 *
 * A list of error severities can be seen in the description of @ref oriErrorCallback.
 *
 * The specified error callback (if none is given, then the default one) will be called when a debug message
 * that matches the specified criteria is enqueued by the Orion library.
 *
 * @param severities a bit field of severities to enable
 *
 * @sa oriErrorCallback
 *
 * @ingroup group_Errors
 *
 */
void oriEnableDebugMessages(oriErrorSeverity severities);



// ============================================================================
// *****        LIBRARY MANAGEMENT                                        *****
// ============================================================================

// library flags

typedef enum oriLibraryFlag {
    TEMP = 0x0
} oriLibraryFlag;

/**
 * @brief Set a library-wide flag or value
 *
 * This function can be used to set a library-wide flag to configure your application.
 * The flags that can be set can be seen below.
 *
 * | Flag name  | Description  | Available values | Default value |
 * | ---------- | ------------ | ---------------- | ------------- |
 * | @c temp    | temp         | temp             | temp          |
 *
 * @param flag the flag to update
 * @param val the value to set the flag to
 *
 * @ingroup group_Meta
 *
 */
void oriSetFlag(oriLibraryFlag flag, unsigned int val);



// ============================================================================
// *****        STATE                                                     *****
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
typedef struct oriState oriState;

/**
 * @brief Create an Orion state object and return its handle.
 *
 * For the @c apiVersion parameter, you should use the @c VK_MAKE_API_VERSION macro defined in the Vulkan header.
 *
 * @param apiVersion the Vulkan version to use, as specified in the
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-coreversions-versionnumbers">Specification</a>.
 *
 * @return the resulting Orion state handle.
 *
 * @sa oriState
 * @sa oriFreeState()
 *
 * @ingroup group_Meta
 *
 */
oriState *oriCreateState(unsigned int apiVersion);

/**
 * @brief Destroy the specified Orion state.
 *
 * Calling this function clears all data stored in @c state and destroys all Orion objects registered to it.
 *
 * @param state handle to the state object to be destroyed.
 *
 * @sa oriState
 *
 * @ingroup group_Meta
 *
 */
void oriFreeState(oriState *state);

/**
 * @brief Set application info for a state object.
 *
 * The @c apiVersion parameter of the application info is set in oriCreateState().
 *
 * @note The @c version and @c engineVersion parameters must be formatted as in the
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-coreversions-versionnumbers">Vulkan Specification</a>.
 *
 * @param state the state the object is to be registered into
 * @param ext equivalent to the @c pNext parameter in the Vulkan Specification (linked below): NULL or a pointer to a structure extending this structure.
 * @param name NULL, or a string containing the name of the application.
 * @param version the version of the application.
 * @param engineName NULL, or a string containing the name of the engine used to create the application.
 * @param engineVersion the version of the engine used to to create the application.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html">Vulkan Docs/VkApplicationInfo</a>
 *
 * @ingroup group_Meta
 *
 */
void oriSetStateApplicationInfo(oriState *state, const void *ext, const char *name, unsigned int version, const char *engineName, unsigned int engineVersion);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_H
