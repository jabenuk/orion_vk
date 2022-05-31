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
 * @file error.c
 * @author jack bennett
 * @brief Error handling functions
 *
 * @copyright Copyright (c) 2022
 *
 * This file contains functions related to error handling.
 *
 * It should be noted that this mostly refers to library debugging - that is to say, debugging aspects of the
 * Orion library rather than the Vulkan API specifically. Although some functionality related to Vulkan
 * debugging is indeed located in this file, but this is generally when it is directly tied to creation of state
 * or other library features.
 *
 */

#include "orion.h"
#include "orion_funcs.h"
#include "orion_structs.h"



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PRIVATE FUNCTIONALITY                               *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

// declared in headers/orion_funcs.h
void _ori_ThrowError(const char *name, unsigned int code, const char *message, oriErrorSeverityBit severity) {
    // check if the error is meant to be displayed
    if ((_orion.displayedErrorSeverities & severity) != severity) {
        return;
    }

    // set the default error callback if the current one is NULL
    if (!_orion.callbacks.errorCallback) {
        _orion.callbacks.errorCallback = _ori_DefaultErrorCallback;
    }

    // throw error to callback
    _orion.callbacks.errorCallback(
        name,
        code,
        message,
        severity,
        _orion.callbacks.errorCallbackUserData
    );
}



// ============================================================================
// ----------------------------------------------------------------------------
// *****        ORION PUBLIC INTERFACE                                    *****
// ----------------------------------------------------------------------------
// ============================================================================



// ============================================================================
// *****        LIBRARY ERROR HANDLING                                    *****
// ============================================================================

/**
 * @brief Set the global Orion error callback function.
 *
 * User data can be passed to the callback function (@ref oriErrorCallback) through the @c pointer
 * parameter. This will be recieved in the callback with the parameter of the same name.
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
void oriSetErrorCallback(oriErrorCallback callback, void *pointer) {
    _orion.callbacks.errorCallback = callback;
    _orion.callbacks.errorCallbackUserData = pointer;
}

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
void oriEnableLibDebugMessages(oriErrorSeverityBit severities) {
    _orion.displayedErrorSeverities |= severities;
}
