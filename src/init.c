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

#include "orion.h"
#include "internal.h"

#include <vulkan/vulkan.h>

#include <stdlib.h>
#include <string.h>

// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================

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
oriState *oriCreateState(unsigned int apiVersion) {
    oriState *r = malloc(sizeof(oriState));

    // init to ensure everything (most importantly linked list heads) start at NULL
    memset(r, 0, sizeof(oriState));

    r->apiVersion = apiVersion;

    return r;
}

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
void oriFreeState(oriState *state) {
    free(state);
}

/**
 * @brief Set application info for a state object.
 *
 * The @c apiVersion parameter of the application info is set in oriCreateState().
 *
 * It is not required to use this function (and hence VkApplicationInfo), but it is recommended so as to support what Vulkan calls 'driver optimisations'.
 * Whatever that means.
 *
 * @param state the state the object is to be registered into
 * @param ext equivalent to the @c pNext parameter in the Vulkan Specification (linked below): NULL or a pointer to a structure extending this structure.
 * @param name NULL, or a string containing the name of the application.
 * @param version the version of the application.
 * @param engineName NULL, or a string containing the name of the engine used to create the application.
 * @param engineVersion the version of the engine used to to create the application.
 * @param apiVersion the highest version of Vulkan that the application is to use.
 *
 * @sa <a href="https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html">Vulkan Docs/VkApplicationInfo</a>
 *
 * @ingroup group_Meta
 *
 */
void oriSetStateApplicationInfo(oriState *state, const void *ext, const char *name, unsigned int version, const char *engineName, unsigned int engineVersion) {
    // using a compound literal should (?) be far less expensive than constantly dereferencing state to redefine the properties separately.
    state->appInfo = (VkApplicationInfo) {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        ext,
        name,
        version,
        engineName,
        engineVersion,
        state->apiVersion
    };
}
