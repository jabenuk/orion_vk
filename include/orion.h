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
