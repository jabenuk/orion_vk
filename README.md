<img src=".github/graphics/icon.png" align=left height=100px>

# orion_vk

[![Build HTML documentation](https://github.com/kosude/orion_vk/actions/workflows/docs-build.yml/badge.svg)](https://github.com/kosude/orion_vk/actions/workflows/docs-build.yml)
[![Licence](https://img.shields.io/github/license/kosude/orion_vk)](/LICENCE)

A low-level Vulkan graphics library for C that aims to abstract the more explicit
aspects of the Vulkan API without losing any functionality.

## Documentation

For library documentation, see the
[GitHub pages deployment](https://kosude.github.io/orion_vk/). Due to being very
early in development, this documentation  is **_very_ subject to change**.

## Dependencies

Dependencies listed below (with the exception of the [Vulkan SDK](#vulkan-sdk))
are included in the repository, as submodules where possible.

Click the **link** <img src=".github/graphics/symbolic/link/external.svg" width=14px>
icon to go to a dependency's **website**, click the **GitHub**
<img src=".github/graphics/symbolic/link/github.svg" width=14px> icon to go to its
**repository**, or click the **folder**
<img src=".github/graphics/symbolic/link/local.svg" width=14px> icon to go to
its **directory or submodule within this repository**.

I am not associated with any of the developers of any of the libraries used
by this project.

### uthash [<img src=".github/graphics/symbolic/link/external.svg" width=16px>](https://troydhanson.github.io/uthash/) [<img src=".github/graphics/symbolic/link/github.svg" width=16px>](https://github.com/troydhanson/uthash) [<img src=".github/graphics/symbolic/link/local.svg" width=16px>](/deps/core/)
Orion uses the **uthash** header for efficiently storing pointers to Orion-created
Vulkan structures, as well as some other internal functionality.

uthash is licensed under a revised BSD licence. For more information, see
their GitHub repository or the web documentation.

### Vulkan SDK [<img src=".github/graphics/symbolic/link/external.svg" width=16px>](https://vulkan.lunarg.com/)
You must have the **Vulkan SDK** installed when developing applications with
Orion or Vulkan in general. It can be downloaded from
[LunarG](https://vulkan.lunarg.com/) - if you are on Linux, you should install
it via your package manager if possible. Here's the
relevant [Arch Wiki entry](https://wiki.archlinux.org/title/Vulkan#Installation),
for example.

### GLFW [<img src=".github/graphics/symbolic/link/external.svg" width=16px>](https://www.glfw.org/) [<img src=".github/graphics/symbolic/link/github.svg" width=16px>](https://github.com/glfw/glfw) [<img src=".github/graphics/symbolic/link/local.svg" width=16px>](/deps/devel/) _(for tests and examples)_

[Test](tests/) and [example](examples/) projects make use of **GLFW** for
managing windows.

GLFW is licensed under the zlib/libpng licence. For more information about using
it in your projects, see their website or GitHub repository.
