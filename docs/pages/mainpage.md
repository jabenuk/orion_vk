# Home                          {#mainpage}

Source code: [kosude/orion](https://github.com/kosude/orion_vk)


## Summary                      {#sct_mainpage_summary}

The original goal of Orion was to run OpenGL without the annoyances of raw
OpenGL code, but it wasn't (and still isn't) really intended to be used in
production or actual development. It's just a fun project for me to work on
in my own time.

Now, I'm shifting my focus towards Vulkan, due to its (somewhat ridiculous,
in a fun way) explicit-ness and its priority on performance and optimisation.
I also think the Vulkan specification is lightyears ahead of the OpenGL
specification - which is pretty abysmal at times - so hopefully development
will be a little less painful. Plus, the new Vulkan design is nice for me - I
prefer having structs with decent names and organisation than having a bunch
of functions (although the OpenGL functions were definitely more simple).

Something else I want to have is actually decent documentation - so we'll
see how that goes.


## Including                    {#sct_mainpage_including}

There is one file to be included to make use of the 'core' library:
[orion.h](@ref orion.h).

This is the public header for including Orion. All public library
functionality is declared here, so the page for this header will have all
core functionality documented, pretty much.


## Documentation contents       {#sct_mainpage_toc}

Functions within the [core library](@ref grp_core) have been organised into
the following modules:
    - [Library management](@ref grp_core_man): _functions that handle the
    initialisation, management, and eventual termination of the library._
    - [Error handling](@ref grp_core_errors): _functionality to catch errors
    thrown by Orion at runtime._
    - [Vulkan API abstractions](@ref grp_core_vkapi) _abstractions of the
    Vulkan API._
        - [Core Vulkan API](@ref grp_core_vkapi_core) _core, general
        Vulkan functionality._
            - [Device management](@ref grp_core_vkapi_core_devices): _Vulkan
            device management._
        - [Layers and extensions](@ref grp_core_vkapi_ext) _Vulkan
        extensibility._

For information about debugging, see the [Debugging](@ref debugging) page.
It might also be helpful to see the full list of error codes and function
return enums, which you can find in the [Error index](@ref errors).


## Applying library-wide optimisations {#sct_mainpage_radop}

Optionally, you may build Orion with the `-DORION_RADOP=ON` CMake flag. This
disables some features that may need more processing power despite not
necessarily being used, such as library debug output (see
[Debugging/Disabling debug output](@ref sct_debugging_disabling) for more
information in this case).

This can certainly be very limiting in some cases, hence the name 'RADOP';
<b>rad</b>ical <b>op</b>timisation - and it is normally not necessary.
