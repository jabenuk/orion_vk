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
 * @author jack bennett
 * @brief The public header of the core Orion library.
 *
 * @copyright Copyright (c) 2022
 *
 * This is the public header for including Orion.
 *
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
 * Note that Vulkan objects, like <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html">instances</a>,
 * are @b not stored inside oriState, but rather this struct contains @b handles to said Vulkan objects, so that it can manage them.
 * For example, you can create a Vulkan instance using data from an oriState object with oriCreateInstance(), but the function @b returns
 * the instance so you can deal with it on your end. However, the function also returns the instance to the state, so that, when you call
 * oriFreeState(), the instance (and any other instances made with this state) will implicitly be freed.
 *
 * @sa oriCreateState()
 * @sa oriFreeState()
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html">Vulkan Docs/VkInstance</a>
 *
 * @ingroup group_Meta
 *
 */
typedef struct oriState oriState;



// ============================================================================
// *****        ENUMS                                                     *****
// ============================================================================

/**
 * @brief Library flag that can be set to a desired value.
 *
 * Library flags can be updated with oriSetFlag() or an equivalent function for other types.
 *
 * For a comprehensive list of available library flags, see the @ref section_main_Config "Home/Configuration" section.
 *
 * @ingroup group_Meta
 *
 */
typedef enum oriLibraryFlag {
    ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS =    1
} oriLibraryFlag;

/**
 * @brief The severity of a debug message.
 *
 * See the @ref section_Debugging_ErrorCallback "Debugging/Error callback" section for more information about how
 * Orion errors and other debug messages are categorised.
 *
 * @ingroup group_Errors
 *
 */
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
    ORION_RETURN_STATUS_ERROR_INVALID_ENUM = 3,
    ORION_RETURN_STATUS_MEMORY_ERROR = 4,
    ORION_RETURN_STATUS_EXT_NOT_ENABLED = 5,
    ORION_RETURN_STATUS_LAYER_NOT_ENABLED = 6
} oriReturnStatus;



// ============================================================================
// *****        CORE VULKAN API ABSTRACTIONS                              *****
// ============================================================================

/**
 * @brief Apply all previous properties set to the given @c state and create a VkInstance object, managed by @c state.
 *
 * Properties previously passed to @c state in functions such as oriDefineStateApplicationInfo(), and features specified in functions like oriFlagLayerEnabled(), will
 * be applied and used to create a <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html">VkInstance</a> object.
 *
 * The resulting instance object will be (memory) managed by the state.
 *
 * @param state the state object from which properties will be used, and to which the resulting VkInstance will be tied.
 * @param ext equivalent to the @c pNext parameter in the Vulkan Specification (linked below): NULL or a pointer to a structure extending this structure.
 * @param instancePtr a pointer to the structure to which the instance will be returned.
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html">Vulkan Docs/VkInstance</a>
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateInfo.html">Vulkan Docs/VkInstanceCreateInfo</a>
 *
 * @ingroup group_VkAbstractions_Core
 *
 */
oriReturnStatus oriCreateInstance(
    oriState *state,
    const void *ext,
    VkInstance *instancePtr
);

/**
 * @brief Create a Vulkan debug messenger that will be managed by @c state.
 *
 * The callback that will be used by this debug messenger is based on the Orion @ref section_Debugging_ErrorCallback "error callback"; that is to say,
 * a message will be generated by Orion with Vulkan-reported information, such as severity, message, type, etc, and will be reported to the Orion callback in
 * the @c message parameter. The code of the error will be @b 0x03, and the error name will be @b VULKAN_DEBUG_MESSENGER.
 *
 * The user pointer that will be reported to the callback is specified in oriSetErrorCallback(). You do not have to filter the messages yourself.
 *
 * @note The @c VK_EXT_debug_utils extension @b must be enabled; if not, @ref section_ErrorList_FunctionReturns "ORION_RETURN_STATUS_EXT_NOT_ENABLED" will be
 * returned, and @c debugMessengerPtr will be set to @b NULL.
 *
 * @param state the state object to which the resulting
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerEXT.html">VkDebugUtilsMessengerEXT</a> will be tied.
 * @param instance the Vulkan <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html">instance</a> that the messenger
 * will be used with.
 * @param ext equivalent to the @c pNext parameter in the Vulkan Specification (linked below): NULL or a pointer to a structure extending this structure.
 * @param debugMessengerPtr a pointer to the structure to which the debug messenger will be returned.
 * @param severities (bitmask) severities of the messages to @b display.
 * @param types (bitmask) types of the messages to @b display.
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerEXT.html">Vulkan Docs/VkDebugUtilsMessengerEXT</a>
 * @sa <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerCreateInfoEXT.html">Vulkan Docs/VkDebugUtilsMessengerCreateInfoEXT</a>
 *
 * @ingroup group_VkAbstractions_Core_Debugging
 *
 */
oriReturnStatus oriCreateDebugMessenger(
    oriState *state,
    VkInstance *instance,
    const void *ext,
    VkDebugUtilsMessengerEXT *debugMessengerPtr,
    VkDebugUtilsMessageSeverityFlagBitsEXT severities,
    VkDebugUtilsMessageTypeFlagBitsEXT types
);



// ============================================================================
// *****        FEATURE LOADING AND ENABLING                              *****
// ============================================================================

/**
 * @brief Flag the specified Vulkan layer to be enabled when creating instances with oriCreateInstance() using @c state.
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
oriReturnStatus oriFlagLayerEnabled(
    oriState *state,
    const char *layer
);

/**
 * @brief Flag the specified Vulkan instance extension to be enabled when creating instances with oriCreateInstance() using @c state.
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
oriReturnStatus oriFlagInstanceExtensionEnabled(
    oriState *state,
    const char *extension
);

/**
 * @brief Validate any instance extensions in @c state against now-specified layers.
 *
 * This function validates and removes any unavailable or non-existant instance extensions that have been specified for the
 * oriState object @c state.
 *
 * This allows for you to specify instance extensions @b before the layers that provide them - essentially, Vulkan instance extensions are either
 * provided by the implementation or a layer.
 *
 * This function is implicitly called in oriCreateInstance() in case you forget.
 *
 * @warning If you specify instance extensions that are provided by a layer, and you call this function before specifying said layer, these instance
 * extensions will be @b removed from the state's internal list, and therefore will @b not be enabled, even if you specify the layer after this function.
 *
 * @param state the state to validate.
 * @return true if any extensions have been removed from the state's list.
 * @return false if no extensions have been removed.
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriPruneInstanceExtensions(
    oriState *state
);



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
bool oriCheckLayerAvailability(
    const char *layer
);

/**
 * @brief Check if @c layer is enabled in the state object @c state.
 *
 * The layer does not have to be actually 'enabled' yet; the function will still return true if the layer has at least been
 * @b specified (with oriFlagLayerEnabled()) - this means no instances have to have been created with @c state for the function
 * to work properly.
 *
 * You don't need to worry about validating the list of layers; the availability of the specified layers is immediately checked
 * in oriFlagLayerEnabled().
 *
 * @param state the oriState object to query.
 * @param layer the layer to search for.
 * @return true if the layer has been enabled.
 * @return false if the layer has not been enabled.
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriCheckLayerEnabled(
    oriState *state,
    const char *layer
);

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
bool oriCheckInstanceExtensionAvailability(
    const char *extension,
    const char *layer
);

/**
 * @brief Check if the instance extension @c extension is enabled in the state object @c state.
 *
 * Similar to in oriCheckLayerEnabled(), the extension does not have to be actually 'enabled' yet; the function will
 * still return true if it has at least been @b specified (with oriFlagInstanceExtensionEnabled()) - this means no instances
 * have to have been created with @c state for the function to work properly.
 *
 * @b Unlike with layers, however, the validation of the specified instance extensions is not checked outside of
 * oriPruneInstanceExtensions() (which is implicitly called in oriCreateInstance()). If you are calling this function @e before
 * oriCreateInstance(), you should prune the instance extensions first to avoid false positives on invalid extensions. Note, though,
 * that any extensions that are provided by layers not yet specified (even if said layers are specified later) will be
 * @b removed from the state's list of instance extensions. See oriPruneInstanceExtensions() for more.
 *
 * @param state the state object to query.
 * @param extension the instance extension to search for.
 * @return true
 * @return false
 *
 * @ingroup group_VkAbstractions_Layers
 *
 */
bool oriCheckInstanceExtensionEnabled(
    oriState *state,
    const char *extension
);



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
typedef void (* oriErrorCallback)(
    const char *name,
    unsigned int code,
    const char *message,
    oriErrorSeverityBit severity,
    void *pointer
);

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
void oriSetErrorCallback(
    oriErrorCallback callback,
    void *pointer
);

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
void oriEnableLibDebugMessages(
    oriErrorSeverityBit severities
);



// ============================================================================
// *****        LIBRARY MANAGEMENT                                        *****
// ============================================================================

/**
 * @brief Set a library-wide flag or value
 *
 * This function can be used to set a library-wide flag to configure your application.
 *
 * For a comprehensive list of available library flags, see the @ref section_main_Config "Home/Configuration" section.
 *
 * @param flag the flag to update
 * @param val the value to set the flag to
 * @return the return status of the function. If it is not 0 (ORION_RETURN_STATUS_OK) then a problem occurred in the function, and you should check the @ref group_Errors
 * "debug output" for more information.
 *
 * @ingroup group_Meta
 *
 */
oriReturnStatus oriSetFlag(
    oriLibraryFlag flag,
    unsigned int val
);



// ============================================================================
// *****        STATE                                                     *****
// ============================================================================

/**
 * @brief Create an Orion state object and return its handle.
 *
 * @return the resulting Orion state handle.
 *
 * @sa oriState
 * @sa oriFreeState()
 *
 * @ingroup group_Meta
 *
 */
oriState *oriCreateState();

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
void oriFreeState(
    oriState *state
);

/**
 * @brief Set application info to be used by all instances created with the given state object.
 *
 * The @c apiVersion parameter of the application info is set in oriCreateState().
 *
 * @note The @c version and @c engineVersion parameters must be formatted as in the
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-coreversions-versionnumbers">Vulkan Specification</a>.
 *
 * @param state the state the object is to be registered into
 * @param ext equivalent to the @c pNext parameter in the Vulkan Specification (linked below): NULL or a pointer to a structure extending this structure.
 * @param apiVersion the Vulkan version to use, as specified in the
 * <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#extendingvulkan-coreversions-versionnumbers">Specification</a>.
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
void oriDefineStateApplicationInfo(
    oriState *state,
    const void *ext,
    unsigned int apiVersion,
    const char *name,
    unsigned int version,
    const char *engineName,
    unsigned int engineVersion
);

/**
 * @brief Define the properties of messages you want to be displayed by the debug messenger of all instances created with the given state object.
 *
 * This function defines part of the creation info of <b>instance debug messengers</b>, which - if
 * @ref section_main_Config "ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS" is enabled and VK_EXT_debug_utils is specified - are automatically
 * created in oriCreateInstance().
 *
 * This function only has an effect if the VK_EXT_debug_utils extension has been specified. This doesn't necessarily have to be done before this
 * function is called (as long as it is before instance creation) but doing so @e will generate a warning nonetheless. The
 * @ref section_main_Config "ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS" flag must also be enabled - again, this doesn't need to be done before
 * but a warning will be generated if such a scenario were to occur.
 *
 * @note The message filters specified in this function are @b different to those specified in oriEnableLibDebugMessages().
 * Whilst those specified in that function are related to the Orion library, those specified here are related to the Vulkan API.
 *
 * @param state the state to modify.
 * @param severities (bitmask) severities of the messages to @b display. By default, no messages are displayed <i>(even if ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS is true)</i>
 * @param types (bitmask) types of the messages to @b display. By default, no messages are displayed <i>(even if ORION_FLAG_CREATE_INSTANCE_DEBUG_MESSENGERS is true)</i>
 *
 * @sa <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_EXT_debug_utils.html">Vulkan Docs/VK_EXT_debug_utils</a>
 * @sa
 * <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessageSeverityFlagBitsEXT.html">Vulkan Docs/VkDebugUtilsMessageSeverityFlagBitsEXT</a>
 * @sa <a href="https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessageTypeFlagBitsEXT.html">Vulkan Docs/VkDebugUtilsMessageTypeFlagBitsEXT</a>
 *
 * @ingroup group_VkAbstractions_Core_Debugging
 *
 */
void oriSpecifyInstanceDebugMessages(
    oriState *state,
    VkDebugUtilsMessageSeverityFlagBitsEXT severities,
    VkDebugUtilsMessageTypeFlagBitsEXT types
);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_H
