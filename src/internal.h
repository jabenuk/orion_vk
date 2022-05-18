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

#include <stdbool.h>

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
typedef struct oriState {
    // struct of all global linked lists
    struct {
    } listHeads;

    VkApplicationInfo appInfo;
    unsigned int apiVersion;
} oriState;
