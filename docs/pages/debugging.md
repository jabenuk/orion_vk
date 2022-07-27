# Debugging                     {#debugging}

This page refers to 'library' debugging; it is related to Orion errors and
function returns, etc.

For more function-specific documentation, see
[Core Orion functionality/Error handling](@ref grp_core_errors).


## Debug callback               {#sct_debugging_callback}

Orion provides the ability to set your own debug callback, similar to OpenGL,
Vulkan, GLFW, etc, and the library will push errors, warnings, and debugging
information to this callback.

To use the debug callback and recieve debug messages, call the
oriConfigureDebugMessages() function at least once, specifying the types of
messages you want to see. There is a **default, built-in callback function**
which will be implicitly used, or you can set your own callback function
with oriSetDebugCallback().

@note Errors and fatal errors will **always** be output to console, regardless
of debug message configuration.

For information regarding the expected function signature of the callback,
and the meaning of the parameters, see @ref oriDebugCallbackfun. An example of a
custom debug callback function can be seen below:

```c
void debugcbfun(
    const char *name,
    const unsigned int code,
    const char *message,
    const oriSeverityBit_t severity,
    void *pointer
) {
    char severitystr[256];

    switch (severity) {
        case ORION_DEBUG_SEVERITY_VERBOSE_BIT:
            snprintf(severitystr, 256, "LOG");
            break;
        case ORION_DEBUG_SEVERITY_NOTIF_BIT:
            snprintf(severitystr, 256, "NOTIFICATION");
            break;
        case ORION_DEBUG_SEVERITY_WARNING_BIT:
            snprintf(severitystr, 256, "WARNING");
            break;
        case ORION_DEBUG_SEVERITY_ERROR_BIT:
        default:
            snprintf(severitystr, 256, "ERROR");
            break;
        case ORION_DEBUG_SEVERITY_FATAL_BIT:
            snprintf(severitystr, 256, "FATAL!");
            break;
    }

    printf("-----\nmessage: %s\ncode: %d (%s)\nseverity: %s\n", message, code, name, severitystr);
}
```
@note Debug messages of type _VERBOSE_, _NOTIF_, and _WARNING_ do not report
**any** value to the `name` or `code` parameters.

You do not need to do any filtering in your callback, this is handled by the
library already. But, of course, you could filter out specific error codes
or names if you so wish. It's up to you.

The `pointer` parameter allows you to pass any value to your callback. You
can specify this value in the call to oriSetDebugCallback(). Note that this parameter
is not used in the default debug callback.

You can, at any point, set the debug callback to be `NULL`. **This will revert
the program back to using the default, built-in debug callback.** You could
then set your own callback again later.


### Directly calling the debug callback {#sct_debugging_directcall}

Whilst the debug callback is meant for use by the Orion API, you can also call it
yourself using the @ref oriGetDebugCallback() function, which returns the callback.

As a sidenote, you can also retrieve the currently set user data with
@ref oriGetDebugCallbackUserData().


### Using the debug callback with Vulkan debug utils {#sct_debugging_vkdebugutils}

If you are using Vulkan debug utilities
([VK_EXT_debug_utils](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_EXT_debug_utils.html)),
you may wish to call the Orion debug callback to then handle Vulkan messages there.

To [call the debug callback](@ref sct_debugging_directcall) for a Vulkan message,
simply pass `"VULKAN_DEBUG_MESSENGER"` to the `name` parameter, and include
**all** necessary information in the `message` parameter. You should also call
the code as `0xA1`.

Of course, you can change this behaviour if you define your own debug callback.
However, you should still check for `"VULKAN_DEBUG_MESSENGER"` as the name of any
Vulkan-thrown debug messages, as it is what is documented in the
[Error index](@ref errors).

Using this functionality, you could create a Vulkan debug messenger which concatenates
all necessary information - such as severity, type, description, etc - into a string
before calling the Orion debug callback as such:

```c
// this is just an example - `message` is the aforementioned string containing all debug info
// you don't need to set the severity to ERROR, this just makes the message have higher priority
oriGetDebugCallback()("VULKAN_DEBUG_MESSENGER", 0xA1, message, ORION_DEBUG_SEVERITY_ERROR_BIT, NULL);
```


## Error code specifications    {#sct_debugging_errorcodes}

Whilst the contents of _VERBOSE_, _NOTIF_, and _WARNING_ messages are 'magically'
decided by whatever reported them, _ERROR_ and _FATAL_ messages are more
standardised and as such there is a limited amount of potential errors,
quanitifed by their respective codes and names.

For a list of error codes, please see the [Error index](@ref sct_errors_codes).


### Severities                  {#sct_debugging_errorcodes_severities}

Errors are further categorised into severities, which can allow for filtering, which
can be done by bit-masking them when passing to oriConfigureDebugMessages(). No
reported debug messages will have more than one severity bit enabled.

Error severities are represented by @ref oriSeverityBit_t enums.

| List of available error severities     | Description                                                           |
| -------------------------------------- | --------------------------------------------------------------------- |
| ORION_DEBUG_SEVERITY_FATAL_BIT         | Errors that the program @b cannot @b recover @b from.                 |
| ORION_DEBUG_SEVERITY_ERROR_BIT         | Significant but (potentially) recoverable errors.                     |
| ORION_DEBUG_SEVERITY_WARNING_BIT       | Events that may cause problems, but are not directly too significant. |
| ORION_DEBUG_SEVERITY_NOTIF_BIT         | General events, no problems reported.                                 |
| ORION_DEBUG_SEVERITY_VERBOSE_BIT       | Almost @b every event that is happening.                              |

@note When specifying the shown severities in oriConfigureDebugMessages(), you can
specify the `ORION_DEBUG_SEVERITY_ALL_BIT` to specify all debug messages.


## Disabling debug output       {#sct_debugging_disabling}

Outputting debug information causes a lot of string manipulation functions
to be called by Orion, like strncpy() and snprintf(). If you want to avoid
this, you can do so by
[building with radical optimisation enabled](@ref sct_mainpage_radop).

This method stops messages of _VERBOSE_, _NOTIF_ and _WARNING_ type from being
processed. **Errors (message types _ERROR_ and _FATAL_) will always be
processed.**
For information about the debug message types in Orion, see the
[Debug callback](@ref sct_debugging_callback) section below.

@note Vulkan debug messengers are **not** be affected by this macro.


## Function return statuses     {#sct_debugging_returns}

Aside from the debug callback, most Orion functions also return a
@ref oriReturnStatus_t enum that describes how their execution went. This isn't
an advanced debugging tool, but rather a way for you to, for instance,
exit the program if a function does not succeed.

When a function returns a non-OK status, it is usually accompanied by an
appropriate debug message (sent to the debug callback).

For a list of possible function return status enums, see the
[Error index](@ref sct_errors_returns).

@sa [Error index](@ref errors)
@sa [Core/Error handling](@ref grp_core_errors)
