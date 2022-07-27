# Error index                   {#errors}


## Error codes                  {#sct_errors_codes}

This is a list of all errors that may be encountered in Orion.

For information on how these errors are categorised and reported,
see the [Debugging/Debug callback](@ref sct_debugging_callback) and
[Debugging/Error code specifications](@ref sct_debugging_errorcodes)
sections.

| Code       | Name                         | Severity | Description                                                                                                          |
| ---------- | ---------------------------- | -------- | -------------------------------------------------------------------------------------------------------------------- |
| 0xA1       | VULKAN_DEBUG_MESSENGER       | N/a      | A Vulkan debug messenger reported this message. The severity is shown in the description, and is reported by Vulkan. |
| 0x01       | ERR_NULL_POINTER             | Error    | A required parameter was passed as NULL. Function will return @c ORION_RETURN_STATUS_NULL_POINTER.                   |
| 0x02       | ERR_INSTANCE_CREATION_FAIL   | Error    | Vulkan failed to create a VkInstance object                                                                          |
| 0x03       | ERR_NOT_INIT                 | Error    | Library has not been initialised yet (call @ref oriInit())                                                           |
| 0x04       | ERR_INVALID_OBJECT           | Error    | Vulkan object is either invalid or was not created with Orion.                                                       |
| 0x05       | ERR_VULKAN_QUERY_FAIL        | Error    | A Vulkan query function returned a non-OK value.                                                                     |
| 0x06       | ERR_DEVICE_CREATION_FAIL     | Error    | Vulkan failed to create a VkDevice object                                                                            |
| 0xD0       | FERR_MEMORY_ERROR            | Fatal    | A memory error was encountered (e.g. malloc-family function returned null)                                           |


## Function return statuses     {#sct_errors_returns}

This is a list of all oriReturnStatus_t enums that may be returned by most
Orion functions.

For information on how these enums work, see the
[Debugging/Function return statuses](@ref sct_debugging_returns) page.

| List of function return statuses           | Equal to | Description                                                                      |
| ------------------------------------------ | -------- | -------------------------------------------------------------------------------- |
| ORION_RETURN_STATUS_OK                     | 0        | No issues were reported.                                                         |
| ORION_RETURN_STATUS_SKIPPED                | 1        | The function returned early and as such its execution was skipped.               |
| ORION_RETURN_STATUS_NO_OUTPUT              | 2        | All output pointers passed to the function were NULL, so nothing was returned.   |
| ORION_RETURN_STATUS_NULL_POINTER           | 3        | A required parameter was passed as NULL.                                         |
| ORION_RETURN_STATUS_ERROR                  | 4        | The function encountered an error (see debug output stream).                     |
