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
 * @file orion.h
 * @author jack bennett
 * @brief The public header of the core Orion library.
 *
 * @copyright Copyright (c) 2022 jack bennett
 *
 * This is the public header for including Orion, in which all core public
 * library functionality is declared.
 *
 */

#pragma once
#ifndef __ORION_H
#define __ORION_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include <vulkan/vulkan.h> // Vulkan API
#include <stdbool.h> // only about 30 lines, just defines the bool type


// ============================================================================ //
// *****                           Preprocessing                          ***** //
// ============================================================================ //

// if radical optimisation (radop) was enabled at build, no debug output will be processed (no string manipulation functions)
// this greatly reduces debugging capabilities (hence the 'radical'). Errors will still be thrown.
#ifndef ORION_RADOP
#   define __oridebug
#endif

// fallback definition for __func__ for maximum code portability
// (see http://gcc.gnu.org/onlinedocs/gcc-4.8.1/gcc/Function-Names.html)
#if __STDC_VERSION__ < 199901L
#   if __GNUC__ >= 2
#       define __func__ __FUNCTION__
#   else
#       define __func__ "<unknown>"
#   endif
#endif


// ============================================================================ //
// *****                  Orion library public interface                  ***** //
// ============================================================================ //


// ----[Orion library public interface]---------------------------------------- //
//                                   Enums                                      //

/**
 * @brief The severity of a debug message.
 *
 * See the @ref sct_debugging_errorcodes_severities "Debugging/Error code specifications/Severities"
 * section for more information about how Orion errors and other debug messages are categorised.
 *
 * @ingroup grp_core_errors
 *
 */
typedef enum oriSeverityBit_t {
    ORION_DEBUG_SEVERITY_ALL_BIT =      0xFF,   // 0b11111111
    ORION_DEBUG_SEVERITY_FATAL_BIT =    0x01,   // 0b00000001
    ORION_DEBUG_SEVERITY_ERROR_BIT =    0x02,   // 0b00000010
    ORION_DEBUG_SEVERITY_WARNING_BIT =  0x04,   // 0b00000100
    ORION_DEBUG_SEVERITY_NOTIF_BIT =    0x08,   // 0b00001000
    ORION_DEBUG_SEVERITY_VERBOSE_BIT =  0x10    // 0b00010000
} oriSeverityBit_t;

/**
 * @brief The return status of an Orion function.
 *
 * All Orion functions (that could result in errors) return an oriReturnStatus_t enum.
 * If the function returns any value other than @c ORION_RETURN_STATUS_OK (0), something has gone wrong. The name of the returned enum will
 * give some information, but this is generally vague and you should check the @ref debugging "debug output" for more information.
 *
 * This can be used to handle errors in your program.
 *
 * To see a list of all possible return statuses, see the @ref sct_errors_returns "Error index".
 *
 * @ingroup grp_core_errors
 *
 * @sa @ref oriStringifyReturnStatus()
 *
 */
typedef enum oriReturnStatus_t {
    ORION_RETURN_STATUS_OK = 0,
    ORION_RETURN_STATUS_SKIPPED = 1,
    ORION_RETURN_STATUS_NO_OUTPUT = 2,
    ORION_RETURN_STATUS_NULL_POINTER = 3,
    ORION_RETURN_STATUS_ERROR = 4,
} oriReturnStatus_t;


// ----[Orion library public interface]---------------------------------------- //
//                          Function pointer typedefs                           //

/**
 * @brief Callback function for general runtime errors.
 *
 * This function pointer defines a callback function for general runtime errors.
 *
 * It function signature must be followed when setting a custom Orion debug callback, which can
 * be done in oriSetDebugCallback().
 *
 * A list of error severities (passed to the @c severity parameter) can be seen in the
 * [Debugging/Severities](@ref sct_debugging_errorcodes_severities) section.
 *
 * @param name the name of the error ID, if applicable\*
 * @param code the error code, if applicable\*
 * @param message a message for debugging
 * @param severity the severity of the message
 * @param pointer NULL or a user-specified pointer (can be specified in oriSetDebugCallback())
 *
 * \*only applicable when @c severity is _ERROR_ or _FATAL._ Messages with severities of _WARNING_ or below are
 * not standardised, and **do not have a set name or code.**
 *
 * @ingroup grp_core_errors
 *
 * @sa @ref oriSetDebugCallback()
 * @sa @ref oriGetDebugCallback()
 * @sa @ref oriGetDebugCallbackUserData()
 *
 */
typedef void (* oriDebugCallbackfun)(
    const char *name,
    const unsigned int code,
    const char *message,
    const oriSeverityBit_t severity,
    void *pointer
);

/**
 * @brief Suitability check function for physical graphics devices.
 *
 * A function of this signature is to be used to select suitable graphics device/s to be
 * used in the application.
 *
 * Devices reported to this function will only include those with Vulkan support, so if
 * there are no graphics devices with Vulkan support, this function will not be called.
 *
 * You should return @b true if the device supports the extensions you require, the
 * device supports the required queue families, etc. You can make the function as
 * simple or as advanced as you need - just make sure you return @b true if the device
 * is suitable and @b false if it isn't.
 *
 * @param device the device to be queried
 *
 * @ingroup grp_core_vkapi_core_devices
 *
 * @sa @ref oriEnumerateSuitablePhysicalDevices()
 *
 */
typedef bool (* oriPhysicalDeviceSuitabilityCheckfun)(
    const VkPhysicalDevice device
);


// ----[Orion library public interface]---------------------------------------- //
//                             Library management                               //

/**
 * @brief Initialise the library.
 *
 * This function initialises the Orion library.
 *
 * It also initialises a given [VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html) struct (or
 * array of VkInstance structs, depending on the amount given in @c instanceCount.)
 *
 * The parameters @c apiVersion, @c applicationName, @c applicationVersion, @c engineName, and @c engineVersion will be passed to Vulkan via the creation
 * of a [VkApplicationInfo](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html) structure.
 *
 * If @c enabledLayerCount is not 0, then @c enabledLayers must be an array of null-terminated strings containing the names of each layer to enable for
 * the instance(s) (if @c enabledLayerCount is 1, then you can simply pass a pointer to a string). This logic also applies to the @c enabledInstanceExtensionCount
 * and @c enabledInstanceExtensions parameters.
 *
 * @note Specifying the same layer or extension multiple times @b will cause problems, as duplicates are not accounted for. It is on you to avoid
 * duplicates.
 *
 * @param instanceCount the amount of instances to create. This should be 1 in almost all cases, and <b>cannot be 0</b>.
 * @param instanceOut if `instanceCount` was 1, then this is a pointer to the VkInstance struct to initialise. Otherwise, it is an array of VkInstance structs.
 * @param instanceFlags a bitmask of [VkInstanceCreateFlagBits](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateFlagBits.html)
 * indicating the collective behaviour of the instances.
 * @param apiVersion the Vulkan API version.
 * @param applicationName NULL or the name of the application.
 * @param applicationVersion the version of the application.
 * @param engineName NULL or the name of the engine.
 * @param engineVersion the version of the engine.
 * @param enabledLayerCount the amount of layers to enable for the instances.
 * @param enabledLayers an array of the names of the layers to enable for the instances.
 * @param enabledInstanceExtensionCount the amount of extensions to enable for the instances.
 * @param enabledInstanceExtensions an array of the names of the extensions to enable for the instances.
 * @param instanceNext NULL or a pointer to a structure to extend the instance creation info structures.
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 * @return [SKIPPED](@ref oriReturnStatus_t) if the library was already initialised or if an instance was already created to @c instanceOut
 * @return [NO_OUTPUT](@ref oriReturnStatus_t) if @c instanceOut is NULL
 * @return [NULL_POINTER](@ref oriReturnStatus_t) if @c enabledLayers is NULL but @c enabledLayerCount is more than 0, or for the @c enabledInstanceExtension* equivalents
 * @return [ERROR](@ref oriReturnStatus_t) if there was an unspecified error, or if memory for the instance failed to allocate
 *
 * @ingroup grp_core_man
 *
 * @sa [Vulkan Docs/VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html)
 * @sa [Vulkan Docs/VkInstanceCreateFlagBits](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateFlagBits.html)
 * @sa @ref oriTerminate()
 *
 */
const oriReturnStatus_t oriInit(
    const unsigned int instanceCount,
    VkInstance *instanceOut,
    const VkInstanceCreateFlags instanceFlags,
    const unsigned int apiVersion,
    const char *applicationName,
    const unsigned int applicationVersion,
    const char *engineName,
    const unsigned int engineVersion,
    const unsigned int enabledLayerCount,
    const char **enabledLayers,
    const unsigned int enabledInstanceExtensionCount,
    const char **enabledInstanceExtensions,
    const void *instanceNext
);

/**
 * @brief Terminate the library and destroy the instance that was created with @ref oriInit().
 *
 * This function terminates the Orion library as well as @b destroying the instance that was previously created
 * using @ref oriInit().
 *
 * You should be able to initialise the library again after calling this function.
 *
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 *
 * @ingroup grp_core_man
 *
 * @sa @ref oriInit()
 *
 */
const oriReturnStatus_t oriTerminate();

/**
 * @brief Optionally define the memory allocation functions to be used in Vulkan functions.
 *
 * This optional function sets the internally-held structure in which allocation function pointers can be defined.
 *
 * The structure passed to the @c callbacks parameter of this function will be referenced in any Vulkan function called internally by
 * Orion with a @c pAllocator parameter.
 *
 * Passing @b NULL to this function will reset Vulkan to using default allocation callbacks as described by the implementation.
 *
 * You can retrieve these callbacks with @ref oriGetVulkanAllocators().
 *
 * @param callbacks NULL or the callbacks structure to use with Vulkan functions.
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 *
 * @ingroup grp_core_man
 *
 * @sa [Vulkan Docs/Memory allocation](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#memory-allocation)
 * @sa [Vulkan Docs/VkAllocationCallbacks](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkAllocationCallbacks.html)
 * @sa @ref oriGetVulkanAllocators()
 *
 */
const oriReturnStatus_t oriSetVulkanAllocators(
    VkAllocationCallbacks *callbacks
);

/**
 * @brief Retrieve the memory allocation functions used in Vulkan functions.
 *
 * This function gets the internally-held structure in which allocation function pointers are defined.
 * This structure can be set with @ref oriSetVulkanAllocators().
 *
 * If no custom memory allocation functions have been set, this function will return NULL.
 *
 * @return the callbacks structure used internally with Vulkan functions.
 *
 * @ingroup grp_core_man
 *
 * @sa @ref oriSetVulkanAllocators()
 *
 */
const VkAllocationCallbacks *oriGetVulkanAllocators();


// ----[Orion library public interface]---------------------------------------- //
//                                   Debugging                                  //

/**
 * @brief Enable Orion debug output and recieve any messages that fall under the specified criteria.
 *
 * This function enables Orion debug output with the specified criteria.
 *
 * A list of error severities can be seen in the
 * [Debugging/Severities](@ref sct_debugging_errorcodes_severities) section.
 *
 * The debug callback specified with oriSetDebugCallback() (or the default one if this was not done)
 * will be called when a debug message that matches the specified criteria is enqueued by the Orion library.
 *
 * You may call this function multiple times throughout runtime - the severities that will be shown
 * will be overriden each time the function is executed.
 *
 * @param severities a bit field of @ref oriSeverityBit_t enumerators.
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 *
 * @ingroup grp_core_errors
 *
 * @sa @ref oriDebugCallbackfun
 *
 */
const oriReturnStatus_t oriConfigureDebugMessages(
    const oriSeverityBit_t severities
);

/**
 * @brief Set the global Orion [debug callback](@ref sct_debugging_callback) function.
 *
 * This function sets the global Orion [debug callback](@ref sct_debugging_callback) function.
 *
 * User data can be passed to the callback through the @c pointer parameter. This will be
 * recieved in the callback with the parameter of the same name.
 *
 * Pass NULL to the @c callback parameter to use the default, built-in debug callback.
 *
 * Pass NULL to the @c pointer parameter if you do not want to pass any data to the callback.
 *
 * @param callback NULL or the callback function to use, definition specified as @ref oriDebugCallbackfun.
 * @param pointer NULL or specified user data that will be sent to the callback.
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 *
 * @ingroup grp_core_errors
 *
 * @sa @ref oriDebugCallbackfun
 * @sa @ref oriGetDebugCallback()
 * @sa @ref oriGetDebugCallbackUserData()
 *
 */
const oriReturnStatus_t oriSetDebugCallback(
    const oriDebugCallbackfun callback,
    void *pointer
);

/**
 * @brief Get the current Orion [debug callback](@ref sct_debugging_callback) function.
 *
 * This function retrieves the current Orion [debug callback](@ref sct_debugging_callback) function. You
 * may use this in order to make direct calls to said callback function.
 *
 * If you have not set a debug callback (see @ref oriSetDebugCallback()), this function will return the built-in default debug callback.
 *
 * @return the current debug callback function
 *
 * @ingroup grp_core_errors
 *
 * @sa @ref oriDebugCallbackfun
 * @sa @ref oriSetDebugCallback()
 * @sa @ref oriGetDebugCallbackUserData()
 *
 */
const oriDebugCallbackfun oriGetDebugCallback();

/**
 * @brief Get the current user data that was set to be passed to the Orion [debug callback](@ref sct_debugging_callback) function.
 *
 * This function retrieves the current user data that was set to be passed to the Orion [debug callback](@ref sct_debugging_callback).
 *
 * If you have not set any user data (see @ref oriSetDebugCallback()), this function will return NULL.
 *
 * @return the current debug callback user data
 *
 * @ingroup grp_core_errors
 *
 * @sa @ref oriDebugCallbackfun
 * @sa @ref oriGetDebugCallback()
 * @sa @ref oriSetDebugCallback()
 *
 */
const void *oriGetDebugCallbackUserData();

/**
 * @brief Convert an oriReturnStatus_t enum into a more descriptive string.
 *
 * This function converts an @ref oriReturnStatus_t enum into a more descriptive string.
 *
 * Keep in mind that [return statuses](@ref sct_debugging_returns) are not meant to be very descriptive, as they serve simply to
 * give a vague idea of whether or not a function succeeded in executing.
 *
 * An example of a described enum is `ORION_RETURN_STATUS_OK` = <i>"function executed successfully"</i>.
 *
 * @param status the @ref oriReturnStatus_t enumerator to stringify
 * @return a string that describes the `status` enumerator
 *
 * @ingroup grp_core_errors
 *
 * @sa @ref oriReturnStatus_t
 *
 */
const char *oriStringifyReturnStatus(
    const oriReturnStatus_t status
);


// ----[Orion library public interface]---------------------------------------- //
//                    Vulkan extensions and feature loading                     //

/**
 * @brief Check if the given layer is provided by the Vulkan implementation.
 *
 * This function queries if the given layer is provided by the Vulkan implementation, returns true if
 * it is, and returns false if it is not.
 *
 * @b False will be returned in the event of an error.
 *
 * @param layer the name of the layer to check.
 * @return true if the layer is provided.
 * @return false if the layer is @b not provided, <b>or if there was an error.</b>
 *
 * @ingroup grp_core_vkapi_ext
 *
 * @sa @ref oriCheckLayerEnabled()
 * @sa @ref oriEnumerateEnabledLayers()
 *
 */
const bool oriCheckLayerAvailability(
    const char *layer
);

/**
 * @brief Check if the given layer is enabled for the specified instance.
 *
 * This function checks if the given layer is enabled for the specified
 * [VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html) object.
 *
 * @param instance the [VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html) object to query.
 * @param layer the name of the layer to search for
 * @return true if the layer was found
 * @return false if the layer was @b not found, <b>or if there was an error.</b>
 *
 * @ingroup grp_core_vkapi_ext
 *
 * @sa [Vulkan Docs/VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html)
 * @sa @ref oriCheckLayerAvailability()
 * @sa @ref oriEnumerateEnabledLayers()
 *
 */
const bool oriCheckLayerEnabled(
    const VkInstance *instance,
    const char *layer
);

/**
 * @brief Retrieve the array of enabled layers for the specified instance.
 *
 * This function retrieves the array of enabled layers for the specified instance.
 *
 * @param instance the [VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html) object to query.
 * @param layerCountOut NULL or a pointer to the variable into which the amount of layers will be returned
 * @param layerNamesOut NULL or a pointer to the array of strings into which the list of layer names will be returned
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 * @return [NULL_POINTER](@ref oriReturnStatus_t) if @c instance is NULL
 * @return [NO_OUTPUT](@ref oriReturnStatus_t) if both @c layerCountOut @b and @c layerNamesOut are NULL
 * @return [ERROR](@ref oriReturnStatus_t) if @c instance was invalid or if it was not created with Orion
 *
 * @ingroup grp_core_vkapi_ext
 *
 * @sa [Vulkan Docs/VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html)
 * @sa @ref oriCheckLayerAvailability()
 * @sa @ref oriCheckLayerEnabled()
 *
 */
const oriReturnStatus_t oriEnumerateEnabledLayers(
    const VkInstance *instance,
    unsigned int *layerCountOut,
    char ***layerNamesOut
);

/**
 * @brief Check if the given instance extension is provided by either the Vulkan implementation or the given layer.
 *
 * This function queries if the given instance extension is provided by either the Vulkan implementation or the given layer,
 * returns true if it is, and returns false if otherwise.
 *
 * If @c layer is NULL, Orion will check if the extension is provided by your machine's Vulkan implementation.
 * Otherwise, Orion will check if the extension is provided by @c layer.
 *
 * @b False will be returned in the event of an error, or if @c layer is invalid.
 *
 * @param extension the name of the instance extension to check.
 * @param layer NULL or the name of the providing layer.
 * @return true if the extension is provided.
 * @return false if the extension is @b not provided, <b>or if there was an error.</b>
 *
 * @ingroup grp_core_vkapi_ext
 *
 * @sa @ref oriCheckInstanceExtensionEnabled()
 * @sa @ref oriEnumerateEnabledInstanceExtensions()
 *
 */
const bool oriCheckInstanceExtensionAvailability(
    const char *extension,
    const char *layer
);

/**
 * @brief Check if the given instance extension is enabled for the specified instance.
 *
 * This function checks if the given instance extension is enabled for the specified
 * [VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html) object.
 *
 * @param instance the [VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html) object to query.
 * @param extension the name of the instance extension to search for
 * @return true if the instance extension was found
 * @return false if the instance extension was @b not found, <b>or if there was an error.</b>
 *
 * @ingroup grp_core_vkapi_ext
 *
 * @sa [Vulkan Docs/VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html)
 * @sa @ref oriCheckInstanceExtensionAvailability()
 * @sa @ref oriEnumerateEnabledInstanceExtensions()
 *
 */
const bool oriCheckInstanceExtensionEnabled(
    const VkInstance *instance,
    const char *extension
);

/**
 * @brief Retrieve the array of enabled instance extensions for the specified instance.
 *
 * This function retrieves the array of enabled instance extensions for the specified instance.
 *
 * @param instance the [VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html) object to query.
 * @param extCountOut NULL or a pointer to the variable into which the amount of instance extensions will be returned
 * @param extNamesOut NULL or a pointer to the array of strings into which the list of instance extension names will be returned
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 * @return [NULL_POINTER](@ref oriReturnStatus_t) if @c instance is NULL
 * @return [NO_OUTPUT](@ref oriReturnStatus_t) if both @c extCountOut @b and @c extNamesOut are NULL
 * @return [ERROR](@ref oriReturnStatus_t) if @c instance was invalid or if it was not created with Orion
 *
 * @ingroup grp_core_vkapi_ext
 *
 * @sa [Vulkan Docs/VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html)
 * @sa @ref oriCheckInstanceExtensionAvailability()
 * @sa @ref oriCheckInstanceExtensionEnabled()
 *
 */
const oriReturnStatus_t oriEnumerateEnabledInstanceExtensions(
    const VkInstance *instance,
    unsigned int *extCountOut,
    char ***extNamesOut
);

/**
 * @brief Check if the given device extension is provided by either the Vulkan implementation or the given layer.
 *
 * This function queries if the given device extension is provided by either the Vulkan implementation or the specified layer,
 * returns true if it is, and returns false if otherwise.
 *
 * If @c layer is NULL, Orion will check if the extension is provided by your machine's Vulkan implementation.
 * Otherwise, Orion will check if the extension is provided by @c layer.
 *
 * @b False will be returned in the event of an error, or if @c layer is invalid.
 *
 * @param physicalDevice the [VkPhysicalDevice](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html)
 * to which the extension may be available.
 * @param extension the name of the instance extension to search for
 * @param layer NULL or the name of the providing layer.
 * @return true if the extension is available to @c physicalDevice.
 * @return false if the extension is @b not available to @c physicalDevice, <b>or if there was an error.</b>
 *
 * @ingroup grp_core_vkapi_ext
 *
 * @sa [Vulkan Docs/VkPhysicalDevice](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html)
 *
 */
const bool oriCheckDeviceExtensionAvailability(
    const VkPhysicalDevice physicalDevice,
    const char *extension,
    const char *layer
);


// ----[Orion library public interface]---------------------------------------- //
//                          Vulkan device management                            //

/**
 * @brief Create a logical device to connect to one or more physical devices.
 *
 * This function creates a logical device ([VkDevice](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDevice.html))
 * to connect to a physical device. The resulting logical device will be managed by Orion.
 *
 * The @c deviceNext parameter can be used to extend the device structure. For instance, you could use it to add a
 * [VkDeviceGroupDeviceCreateInfo](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceGroupDeviceCreateInfo.html),
 * which would allow you to connect the logical device to @b multiple physical devices.
 *
 * See [Vulkan Docs/VkDeviceCreateInfo](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html)
 * and, occasionally,
 * [Vulkan Docs/VkDeviceGroupDeviceCreateInfo](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceGroupDeviceCreateInfo.html)
 * for more info.
 * @param deviceOut pointer to the [VkDevice](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDevice.html)
 * into which the resulting device will be returned.
 * @param deviceFlags a bitmask of [VkDeviceCreateFlags](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateFlags.html)
 * indicating the behaviour of the logical device.
 * @param physicalDevice the physical device to interface with.
 * @param queueCreateInfoCount the size of the @c queueCreateInfos array.
 * @param queueCreateInfos array of
 * [VkDeviceQueueCreateInfo](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceQueueCreateInfo.html)
 * structures describing the queues to be requested along with the logical device.
 * @param extensionCount the size of the @c extensionNames array.
 * @param extensionNames array of the names of device extensions to be enabled for the logical device.
 * @param enabledFeatures NULL or a pointer to a
 * [VkPhysicalDeviceFeatures](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html)
 * structure containing flags of device features to be enabled.
 * @param deviceNext NULL or a pointer to a structure to extend the device creation info structures.
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 * @return [NULL_POINTER](@ref oriReturnStatus_t) if @c physicalDevice is NULL or if @c queueCreateInfos is NULL but @c queueCreateInfoCount is more than 0, or
 * the extension equivalents
 * @return [SKIPPED](@ref oriReturnStatus_t) if a device has already been allocated by Orion at @c deviceOut.
 * @return [ERROR](@ref oriReturnStatus_t) if the device failed to be created by Vulkan.
 *
 * @ingroup grp_core_vkapi_core_devices
 *
 * @sa [Vulkan Docs/VkDevice](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDevice.html)
 * @sa [Vulkan Docs/VkDeviceCreateInfo](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html)
 * @sa [Vulkan Docs/VkDeviceGroupDeviceCreateInfo](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceGroupDeviceCreateInfo.html)
 * @sa [Vulkan Docs/VkDeviceQueueCreateInfo](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDeviceQueueCreateInfo.html)
 * @sa [Vulkan Docs/VkPhysicalDeviceFeatures](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html)
 *
 */
const oriReturnStatus_t oriCreateLogicalDevice(
    VkDevice *deviceOut,
    const VkDeviceCreateFlags deviceFlags,
    const VkPhysicalDevice physicalDevice,
    const unsigned int queueCreateInfoCount,
    const VkDeviceQueueCreateInfo *queueCreateInfos,
    const unsigned int extensionCount,
    const char **extensionNames,
    VkPhysicalDeviceFeatures *enabledFeatures,
    const void *deviceNext
);

/**
 * @brief Retrieve an array of physical devices accessible to a Vulkan instance that are considered suitable for the application.
 *
 * This function retrieves an array of physical devices accessible to a Vulkan instance that are considered
 * suitable for the application, making use of the @c checkFun function to determine their suitability.
 *
 * If you set @c checkFun to NULL, <b>all available physical devices will be returned.</b>
 *
 * @note If there are no suitable (or available if @c checkFun is NULL) devices, then @c devicesOut will be set to @b NULL.
 * @c countOut will, of course, be set to 0.
 *
 * @warning The @c devicesOut array will be @b allocated by this function and <b>it is up to you to free it.</b>
 *
 * @param instance the [VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html) object to query.
 * @param checkFun NULL, or a @ref oriPhysicalDeviceSuitabilityCheckfun function to determine device suitability on your terms.
 * @param countOut NULL or a pointer to the variable into which the amount of devices will be returned
 * @param devicesOut NULL or a pointer to the array into which the Vulkan physical device objects will be returned
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 * @return [NULL_POINTER](@ref oriReturnStatus_t) if @c instance is NULL
 * @return [NO_OUTPUT](@ref oriReturnStatus_t) if both @c countOut @b and @c devicesOut are NULL
 * @return [ERROR](@ref oriReturnStatus_t) if there was an unspecified error, or if memory for the device failed to allocate
 *
 * @ingroup grp_core_vkapi_core_devices
 *
 * @sa [Vulkan Docs/VkInstance](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkInstance.html)
 * @sa @ref oriPhysicalDeviceSuitabilityCheckfun
 *
 */
const oriReturnStatus_t oriEnumerateSuitablePhysicalDevices(
    const VkInstance *instance,
    const oriPhysicalDeviceSuitabilityCheckfun checkFun,
    unsigned int *countOut,
    VkPhysicalDevice **devicesOut
);

/**
 * @brief Retrieve an array of properties of queue families accessible to a physical device.
 *
 * This function retrieves an array of properties of queue families accessible to the specified
 * [VkPhysicalDevice](https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html) structure.
 *
 * @note If there are no available queue families, then @c familiesOut will be set to @b NULL.
 * @c countOut will, of course, be set to 0.
 *
 * @warning The @c familiesOut array will be @b allocated by this function and <b>it is up to you to free it.</b>
 *
 * @param physicalDevice the [VkPhysicalDevice](https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html) structure
 * to query.
 * @param countOut NULL or a pointer to the variable into which the amount of queue families will be returned
 * @param familiesOut NULL or a pointer to the array into which the Vulkan queue family properties objects will be returned
 * @return [OK](@ref oriReturnStatus_t) if executed successfully
 * @return [NULL_POINTER](@ref oriReturnStatus_t) if @c physicalDevice is NULL
 * @return [NO_OUTPUT](@ref oriReturnStatus_t) if both @c countOut @b and @c familiesOut are NULL
 * @return [ERROR](@ref oriReturnStatus_t) if there was an unspecified or memory error
 *
 * @ingroup grp_core_vkapi_core_devices
 *
 * @sa [Vulkan Docs/VkPhysicalDevice](https://khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html)
 *
 */
const oriReturnStatus_t oriEnumerateAvailableQueueFamilies(
    const VkPhysicalDevice *physicalDevice,
    unsigned int *countOut,
    VkQueueFamilyProperties **familiesOut
);

#ifdef __cplusplus
    }
#endif // __cplusplus

#endif // __ORION_H
