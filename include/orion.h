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
#include <stdbool.h>


// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        STRUCTURES                                                *****
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



// ============================================================================
// *****        ENUMS                                                     *****
// ============================================================================

// library flags
typedef enum oriLibraryFlag {
    TEMP = 0x0
} oriLibraryFlag;

// error severities (bit field):
typedef enum oriErrorSeverityBit {
    ORION_ERROR_SEVERITY_ALL_BIT =      0xFF,   // 0b11111111
    ORION_ERROR_SEVERITY_FATAL_BIT =    0x01,   // 0b00000001
    ORION_ERROR_SEVERITY_ERROR_BIT =    0x02,   // 0b00000010
    ORION_ERROR_SEVERITY_WARNING_BIT =  0x04,   // 0b00000100
    ORION_ERROR_SEVERITY_NOTIF_BIT =    0x08,   // 0b00001000
    ORION_ERROR_SEVERITY_VERBOSE_BIT =  0x10    // 0b00010000
} oriErrorSeverityBit;

/**
 * @brief The return status of an Orion function.
 *
 * All Orion functions (that could result in errors) return an oriReturnStatus enum.
 * If the function returns any value other than @c ORION_RETURN_STATUS_OK (0), something has gone wrong. The name of the returned enum will
 * give some information, but this is generally vague and you should check the @ref group_Errors "debug output" for more information.
 *
 * This can be used to manage errors in your program.
 *
 * @ingroup group_Errors
 *
 */
typedef enum oriReturnStatus {
    ORION_RETURN_STATUS_ERROR_GOOD_LUCK = -1,
    ORION_RETURN_STATUS_OK = 0,
    ORION_RETURN_STATUS_ERROR_NOT_FOUND = 1,
    ORION_RETURN_STATUS_ERROR_VULKAN_ERROR = 2,
    ORION_RETURN_STATUS_ERROR_INVALID_ENUM = 3
} oriReturnStatus;



// ============================================================================
// *****        CORE VULKAN API ABSTRACTIONS                              *****
// ============================================================================

/**
 * @brief Apply all previous properties set to the given @c state and create a VkInstance object, stored in @c state.
 *
 * Properties previously passed to @c state in functions such as oriDefineStateApplicationInfo(), and features specified in functions like oriFlagLayerEnabled(), will
 * be applied and used to create a <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html">VkInstance</a> object.
 *
 * The resulting instance object will be managed by the state.
 *
 * @param state the state object from which properties will be used, and to which the resulting VkInstance will be tied.
 * @param instancePtr a pointer to the structure to which the instance will be returned.
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html">Vulkan Docs/VkInstance</a>
 *
 * @ingroup group_VkAbstractions_Core
 *
 */
oriReturnStatus oriCreateStateVkInstance(oriState *state, VkInstance *instancePtr);



// ============================================================================
// *****        FEATURE LOADING AND ENABLING                              *****
// ============================================================================

/**
 * @brief Flag the specified Vulkan layer to be enabled when creating the state's instance with oriCreateStateVkInstance().
 *
 * @param state the state to enable the layer on.
 * @param layer a UTF-8, null-terminated string holding the name of the desired layer.
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @sa oriCheckLayerAvailability()
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
oriReturnStatus oriFlagLayerEnabled(oriState *state, const char *layer);

/**
 * @brief Flag the specified Vulkan instance extension to be enabled when creating the state instance with oriCreateStateVkInstance().
 *
 * For information regarding the difference between 'instance extensions' and 'device extensions' in Vulkan, you should see
 * <a href="https://stackoverflow.com/a/53050492/12980669">this</a> answer on StackOverflow.
 *
 * @note If the given extension is provided by a layer, make sure to flag the layer as enabled with oriFlagLayerEnabled() as well before creating
 * the state instance.
 *
 * @param state the state the enable the extension on.
 * @param extension a UTF-8, null-terminated string holding the name of the desired extension.
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
oriReturnStatus oriFlagInstanceExtensionEnabled(oriState *state, const char *extension);



// ============================================================================
// *****        COMPATIBILITY CHECKS                                      *****
// ============================================================================

/**
 * @brief Return the availability of the specified Vulkan layer.
 *
 * @param layer a UTF-8, null-terminated string holding the name of the desired layer.
 * @return true if the layer is available
 * @return false if the layer is not available
 *
 * @sa oriFlagLayerEnabled()
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriCheckLayerAvailability(const char *layer);

/**
 * @brief Return the availability of the specified Vulkan instance extension.
 *
 * The @c layer parameter is equivalent to the @c pLayerName parameter in the
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkEnumerateInstanceExtensionProperties.html">VkEnumerateInstanceExtensionProperties</a>
 * Vulkan function. This is described in the Specification as the following:
 *  > When pLayerName parameter is NULL, only extensions provided by the Vulkan implementation or by implicitly enabled layers are returned. When pLayerName is the
 *  > name of a layer, the instance extensions provided by that layer are returned.
 *
 * @param extension a UTF-8, null-terminated string holding the name of the desired instance extension.
 * @param layer NULL or a UTF-8 null-terminated string holding the name of the layer to query the extension from.
 * @return true if the instance extension is available
 * @return false if the instance extension is not available
 *
 * @sa oriFlagInstanceExtensionEnabled()
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriCheckInstanceExtensionAvailability(const char *extension, const char *layer);



// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

/**
 * @brief Callback function for general runtime errors
 *
 * This function signature must be followed when creating an error callback for debugging.
 * The global error callback is set in oriSetErrorCallback().
 *
 * For a list of possible error severities, see the @ref page_Debugging "debugging" page.
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
typedef void (* oriErrorCallback)(const char *name, unsigned int code, const char *message, oriErrorSeverityBit severity, void *pointer);

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
 * @param callback NULL or the callback function to use.
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
 * You can pass the @c ORION_ERROR_SEVERITY_ALL_BIT enum to the @c severities parameter to enable all debug messages.
 *
 * @param severities a bit field of severities to enable
 *
 * @sa oriErrorCallback
 *
 * @ingroup group_Errors
 *
 */
void oriEnableDebugMessages(oriErrorSeverityBit severities);



// ============================================================================
// *****        LIBRARY MANAGEMENT                                        *****
// ============================================================================

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
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @ingroup group_Meta
 *
 */
oriReturnStatus oriSetFlag(oriLibraryFlag flag, unsigned int val);



// ============================================================================
// *****        STATE                                                     *****
// ============================================================================

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
void oriDefineStateApplicationInfo(oriState *state, const void *ext, const char *name, unsigned int version, const char *engineName, unsigned int engineVersion);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_H
