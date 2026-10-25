# yggdrasil

**GFX** is a single-header C library for Vulkan 1.3, contained in [`gfx.h`](gfx.h). It
handles the repetitive parts of Vulkan without hiding the API: GFX objects are plain structs
that expose their Vulkan handles, and draws, dispatches and state changes are recorded with
ordinary `vkCmd*` calls.

Summary of features:

- Instance, device and swapchain creation, with validation layers in debug builds.
- Buffers, images and textures suballocated from shared device memory, with batched uploads.
- Shaders whose graphics pipelines keep nearly all state dynamic and are built on demand for
  the attachments they render to. GLSL is compiled at runtime, supports `#include` and hot
  reloading, and descriptor layouts are reflected from the SPIR-V.
- Resources attached to shaders by their GLSL names, with no manual descriptor set handling.
- A render graph that owns the frame's images and records all barriers and layout transitions.
- Ray tracing with acceleration structures and generated shader binding tables.
- Per-frame buffers, GPU timers, debug names and labels, image readback and PNG export.

Windowing, math, cameras and scene loading are outside the scope of GFX. The examples use
GLFW and [vecmat](https://github.com/rikardolajos/vecmat).

## Requirements

- A device supporting Vulkan 1.3. `VK_EXT_extended_dynamic_state3` and
  `VK_EXT_vertex_input_dynamic_state` are used when available. Ray tracing is enabled when
  supported.
- The Vulkan SDK, which provides `glslang`, `SPIRV-Tools`, `SPIRV-Tools-opt` and
  `glslang-default-resource-limits`.
- A C17 compiler on 64-bit Windows, Linux or macOS. MSVC requires `/Zc:preprocessor`.
- Optionally, [stb_image and stb_image_write](https://github.com/nothings/stb) for loading
  textures and writing PNGs.

## Examples

```bash
git clone --recursive https://github.com/rikardolajos/yggdrasil.git
```

```bash
cmake -S yggdrasil -B yggdrasil/build
```

```bash
cmake --build yggdrasil/build
```

Shaders and textures are copied to each example's build directory and loaded by relative
path, so each example runs from its own build directory. F5 reloads shaders and F12 saves
`screenshot.png`.

On macOS, the Vulkan SDK ships two drivers. GFX targets KosmicKrisp, which needs macOS 26 on
Apple Silicon and is selected by pointing `VK_DRIVER_FILES` at the SDK's
`libkosmickrisp_icd.json`. MoltenVK, the SDK's default, is enumerated as well but is not
tested.

### Continuous integration

[`ci.yml`](.github/workflows/ci.yml) builds the examples in debug, with validation layers
and `GFX_EXIT_ON_ERROR`, and runs each of them for ten seconds on Windows and Linux with the
lavapipe software driver. An example passes if it is still running by then, as errors end it
early. A second Linux run builds with `GFX_BAKE_PIPELINE_STATE`. On macOS the examples are
only built, as KosmicKrisp finds no usable GPU in the runners' virtual machines.

| Example | Demonstrates |
| --- | --- |
| [`rasterizer`](examples/rasterizer/main.c) | Minimal setup: textured quad, MSAA, per-frame buffer, single-pass render graph |
| [`shadows`](examples/shadows/main.c) | Two-pass render graph, shadow map sampled with `sampler2DShadow`, GPU timers |
| [`particles`](examples/particles/main.c) | Compute pass and graphics pass sharing a buffer, push constants, blending |
| [`raytracer`](examples/raytracer/main.c) | Acceleration structures, ray tracing pipeline, per-frame TLAS updates |

## Integration

Define `GFX_IMPLEMENTATION` in exactly one C file before including `gfx.h`. Configuration
macros must be defined identically in every file that includes it:

```c
#define GFX_USE_STB_IMAGE       // Optional: gfxCreateTextureFromFile()
#define GFX_USE_STB_IMAGE_WRITE // Optional: gfxSaveImagePNG()
#define GFX_IMPLEMENTATION
#include "gfx.h"
```

Link against Vulkan and the glslang libraries of the SDK, as done in the root
[`CMakeLists.txt`](CMakeLists.txt). The header can be included from C++, but the
implementation must be compiled as C.

## Getting started

GFX keeps a single instance, device and swapchain in the globals `gfxInstance`, `gfxDevice`
and `gfxSwapchain`.
Initialization follows a fixed order:

1. Create a window without a client API.
2. Create the instance with the extensions required by the windowing library.
3. Create a surface from `gfxInstance.instance`.
4. Create the device with the surface. GFX takes ownership of the surface and destroys it
   in `gfxDestroyDevice()`.
5. Create the swapchain with a callback that reports the framebuffer size.

`GFX_DEFAULT_DEVICE` selects the first supported discrete GPU. `gfxEnumerateDevices()` is provided
to list all supported devices available on the system.

A minimal program using GLFW, condensed from the [rasterizer example](examples/rasterizer/main.c).
It draws a triangle whose vertices are generated in the vertex shader from `gl_VertexIndex`:

```c
#define GFX_IMPLEMENTATION
#include "gfx.h"
#include <GLFW/glfw3.h>

static GLFWwindow* pWindow;

// Queried when the swapchain is recreated. Blocks while the window is minimized.
static void framebufferSizeCallback(uint32_t* pWidth, uint32_t* pHeight)
{
    int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);
    while (width == 0 || height == 0) {
        glfwWaitEvents();
        glfwGetFramebufferSize(pWindow, &width, &height);
    }
    *pWidth = (uint32_t)width;
    *pHeight = (uint32_t)height;
}

static void drawPass(VkCommandBuffer cmd, const GfxAttachment* pAttachment, void* pUserData)
{
    GfxShader* pShader = pUserData;
    gfxCmdSetDefaultStates(cmd, pAttachment, 0, NULL, 0, NULL);
    gfxCmdBindShader(cmd, pShader);
    gfxCmdBindResources(cmd, pShader);
    vkCmdDraw(cmd, 3, 1, 0, 0);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    pWindow = glfwCreateWindow(1280, 720, "GFX", NULL, NULL);

    uint32_t extensionCount;
    const char** ppExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    gfxCreateInstance(VK_API_VERSION_1_3, extensionCount, ppExtensions);

    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(gfxInstance.instance, pWindow, NULL, &surface));
    gfxCreateDevice(GFX_DEFAULT_DEVICE, 0, NULL, NULL, surface);
    gfxCreateSwapchain(2, framebufferSizeCallback);

    GfxShaderStageDesc stages[] = {{.pPath = "shader.vert"}, {.pPath = "shader.frag"}};
    GfxShader shader;
    gfxCreateShader(GFX_ARRAY_LEN(stages), stages, &shader);

    // Images with no size follow the swapchain and are recreated on resize
    GfxRenderGraph graph;
    gfxCreateRenderGraph(&graph);
    gfxAddGraphImage(&graph, &(GfxGraphImageDesc){
                                 .pName = "color",
                                 .format = VK_FORMAT_R8G8B8A8_UNORM,
                                 .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                             });
    gfxAddGraphPass(&graph, &(GfxPassDesc){
                                .pName = "Draw",
                                .outputs = {"color"},
                                .function = drawPass,
                                .pUserData = &shader,
                            });
    gfxSetGraphOutput(&graph, "color");

    while (!glfwWindowShouldClose(pWindow)) {
        gfxWaitForFence();

        // A null command buffer means the swapchain was recreated and the frame is skipped
        VkCommandBuffer cmd = gfxAcquireNextImage();
        if (cmd) {
            gfxCmdExecuteRenderGraph(cmd, &graph);
            gfxPresent(cmd, gfxGetGraphImage(&graph, "color"));
        }

        glfwPollEvents();
    }

    gfxDestroyShader(&shader);
    gfxDestroyRenderGraph(&graph);
    gfxDestroySwapchain();
    gfxDestroyDevice();
    gfxDestroyInstance();
    glfwTerminate();
}
```

Shader resources are attached by the instance name of a block, or by the block name when
the block has no instance name, for example `gfxSetShaderBuffer(&shader, "camera", &buffer)`.
Resources are referenced by pointer and must remain at the same address while attached.

### Pipelines and dynamic state

A graphics shader is bound while rendering, in a graphics pass or between
`gfxCmdBeginRendering()` and `gfxCmdEndRendering()`. The first time it is bound for a set of
attachment formats, a pipeline is built for them and kept with the shader.
`gfxCmdSetDefaultStates()` sets every state, and the states that Vulkan 1.3 always has
dynamic, such as depth testing, culling and viewports, are changed with plain `vkCmdSet*`
calls.

Some states are only dynamic with extensions that not every device has, such as KosmicKrisp
on macOS. GFX bakes those into the pipeline instead, so they are set with GFX functions that
take the same arguments as the Vulkan commands and work either way:

| State | Function | Dynamic when |
| --- | --- | --- |
| Primitive topology class | `gfxCmdSetPrimitiveTopology()` | `gfxDevice.primitiveTopologyUnrestricted` |
| Vertex bindings and attributes | `gfxCmdSetVertexInput()` | `gfxDevice.dynamicVertexInput` |
| Sample mask | `gfxCmdSetSampleMask()` | `gfxDevice.dynamicSampleMask` |
| Alpha to coverage | `gfxCmdSetAlphaToCoverageEnable()` | `gfxDevice.dynamicAlphaToCoverage` |
| Polygon mode | `gfxCmdSetPolygonMode()` | `gfxDevice.dynamicPolygonMode` |
| Logic op enable | `gfxCmdSetLogicOpEnable()` | `gfxDevice.dynamicLogicOpEnable` |
| Blend enable, equation and write mask | `gfxCmdSetColorBlendEnable()`, `gfxCmdSetColorBlendEquation()`, `gfxCmdSetColorWriteMask()` | `gfxDevice.dynamicColorBlend` |

The sample count follows the attachments. When one of these states changes after a shader has
been bound, the pipeline for the new state is bound in its place, so they can be set before or
after binding. Setting them before binding avoids building pipelines for combinations that are
only passed through:

```c
gfxCmdSetDefaultStates(cmd, pAttachment, 0, NULL, 0, NULL);
gfxCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
gfxCmdSetColorBlendEnable(cmd, 0, 1, &(VkBool32){VK_TRUE});
gfxCmdBindShader(cmd, &shader);
```

The logic op, depth clamp and alpha to one are dynamic when the device supports it, as
reported by `gfxDevice.dynamicLogicOp`, `gfxDevice.dynamicDepthClamp` and
`gfxDevice.dynamicAlphaToOne`, and are otherwise copy, off and off. Patch control points are
fixed per shader through `GfxShaderStageDesc::patchControlPoints` on the tessellation control
stage.

Buffers, textures, multisampling and depth are covered by the rasterizer example; compute
and ray tracing passes by the particles and raytracer examples.

### Other windowing libraries

Any library that provides the required instance extensions and creates a surface from a
`VkInstance` can replace GLFW. For SDL3, the corresponding calls are
`SDL_CreateWindow()` with `SDL_WINDOW_VULKAN`, `SDL_Vulkan_GetInstanceExtensions()`,
`SDL_Vulkan_CreateSurface()` and `SDL_GetWindowSizeInPixels()`.

## Vulkan interoperability

The Vulkan objects managed by GFX are accessible to application code and third-party
libraries:

| Object | Location |
| --- | --- |
| Instance | `gfxInstance.instance` |
| Physical device, device | `gfxDevice.physicalDevice`, `gfxDevice.device` |
| Queue and queue family | `gfxDevice.queue`, `gfxDevice.queueFamilyIndex` |
| Swapchain format, extent, image count | `gfxSwapchain.format`, `gfxSwapchain.extent`, `gfxSwapchain.imageCount` |
| Buffers and images | `GfxBuffer::buffer`, `GfxImage::image`, `GfxImage::imageView` |
| Attachment formats of a graphics pass | `pAttachment->pipelineRenderingCreateInfo` |

A single queue is used for graphics, compute, transfer and presentation. Libraries that
record with dynamic rendering, such as the Dear ImGui Vulkan backend, can be called from a
graphics pass function.

Additional device extensions and features are passed to `gfxCreateDevice()` and enabled
alongside those GFX requires. Core features must be given in the
`VkPhysicalDeviceVulkan1xFeatures` structures:

```c
VkPhysicalDeviceFeatures2 features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    .features = {.largePoints = VK_TRUE},
};
gfxCreateDevice(GFX_DEFAULT_DEVICE, extensionCount, ppExtensions, &features, surface);
```

Extension entry points used by GFX are cached in `gfxDevice.fn`. Others are loaded with
`VK_LOAD(vkCmdDrawMeshTasksEXT)`, which declares `XvkCmdDrawMeshTasksEXT`. Command buffers
submitted outside GFX should be preceded by `gfxFlushUploads(false)` so that pending uploads
are visible to them.

## Configuration

The following macros are defined before including `gfx.h`:

| Macro | Default | Effect |
| --- | --- | --- |
| `GFX_IMPLEMENTATION` | — | Compiles the implementation, in exactly one file |
| `GFX_USE_STB_IMAGE` | off | Enables `gfxCreateTextureFromFile()` |
| `GFX_USE_STB_IMAGE_WRITE` | off | Enables `gfxSaveImagePNG()` |
| `GFX_REVERSE_Z` | off | Clears depth to 0 and compares with `GREATER` |
| `GFX_MALLOC`, `GFX_REALLOC`, `GFX_FREE` | `malloc`, `realloc`, `free` | Host allocation |
| `GFX_MEMORY_BLOCK_SIZE` | 64 MiB | Size of device memory blocks |
| `GFX_UPLOAD_STAGING_SIZE` | 32 MiB | Staging memory per upload batch |
| `GFX_UPLOAD_BATCHES` | 3 | Upload batches in flight |
| `GFX_MAX_PASS_RESOURCES` | 8 | Inputs and outputs per render graph pass |
| `GFX_MAX_COLOR_ATTACHMENTS` | 8 | Color attachments a graphics pipeline renders to |
| `GFX_MAX_VERTEX_BINDINGS`, `GFX_MAX_VERTEX_ATTRIBUTES` | 8, 16 | Vertex input a pipeline can bake in |
| `GFX_MAX_TIMERS` | 64 | GPU timers per frame |
| `GFX_BAKE_PIPELINE_STATE` | off | Bakes every state that can be baked, to test that path on any device |
| `GFX_EXIT_ON_ERROR` | off | Exits with a failure status on errors instead of breaking into the debugger |

The last two are also CMake options of the examples.

All functions are documented at their declarations in [`gfx.h`](gfx.h).

## License

MIT. See [LICENSE.md](LICENSE.md).
