/* Copyright 2025 Rikard Olajos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Platform
#define GFX_PLATFORM_WINDOWS 1
#define GFX_PLATFORM_LINUX 2

#ifndef GFX_PLATFORM
#if defined(_WIN64)
#define GFX_PLATFORM GFX_PLATFORM_WINDOWS
#elif defined(__linux__)
#define GFX_PLATFORM GFX_PLATFORM_LINUX
#else
#error "Unsupported target platform"
#endif
#endif

#define GFX_WINDOWS (GFX_PLATFORM == GFX_PLATFORM_WINDOWS)
#define GFX_LINUX (GFX_PLATFORM == GFX_PLATFORM_LINUX)

#include <vulkan/vulkan.h>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

// Define GFX_USE_STB_IMAGE if stb_image.h is available. This allows for
// texture loading from file.
#ifdef GFX_USE_STB_IMAGE
#include "stb_image.h"
#endif

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if GFX_LINUX
#include <csignal>
#endif

// Get the length of an array. Don't use for pointers!
#define GFX_ARRAY_LEN(x) (uint32_t)(sizeof(x) / sizeof *(x))

// Memory allocation macros. Overload to use custom allocations.
#ifndef GFX_MALLOC
#define GFX_MALLOC(sz) gfxCheckedMalloc(sz)
#endif

#ifndef GFX_REALLOC
#define GFX_REALLOC(p, sz) realloc(p, sz)
#endif

#ifndef GFX_FREE
#define GFX_FREE(p) free(p)
#endif


// Error handling and logging //

struct VulkanResult {
    VkResult result;
    const char* string;
} gfxVulkanResults[] = {
    {VK_SUCCESS, "VK_SUCCESS"},
    {VK_NOT_READY, "VK_NOT_READY"},
    {VK_TIMEOUT, "VK_TIMEOUT"},
    {VK_EVENT_SET, "VK_EVENT_SET"},
    {VK_EVENT_RESET, "VK_EVENT_RESET"},
    {VK_INCOMPLETE, "VK_INCOMPLETE"},
    {VK_ERROR_OUT_OF_HOST_MEMORY, "VK_ERROR_OUT_OF_HOST_MEMORY"},
    {VK_ERROR_OUT_OF_DEVICE_MEMORY, "VK_ERROR_OUT_OF_DEVICE_MEMORY"},
    {VK_ERROR_INITIALIZATION_FAILED, "VK_ERROR_INITIALIZATION_FAILED"},
    {VK_ERROR_DEVICE_LOST, "VK_ERROR_DEVICE_LOST"},
    {VK_ERROR_MEMORY_MAP_FAILED, "VK_ERROR_MEMORY_MAP_FAILED"},
    {VK_ERROR_LAYER_NOT_PRESENT, "VK_ERROR_LAYER_NOT_PRESENT"},
    {VK_ERROR_EXTENSION_NOT_PRESENT, "VK_ERROR_EXTENSION_NOT_PRESENT"},
    {VK_ERROR_FEATURE_NOT_PRESENT, "VK_ERROR_FEATURE_NOT_PRESENT"},
    {VK_ERROR_INCOMPATIBLE_DRIVER, "VK_ERROR_INCOMPATIBLE_DRIVER"},
    {VK_ERROR_TOO_MANY_OBJECTS, "VK_ERROR_TOO_MANY_OBJECTS"},
    {VK_ERROR_FORMAT_NOT_SUPPORTED, "VK_ERROR_FORMAT_NOT_SUPPORTED"},
    {VK_ERROR_FRAGMENTED_POOL, "VK_ERROR_FRAGMENTED_POOL"},
    {VK_ERROR_UNKNOWN, "VK_ERROR_UNKNOWN"},
    {VK_ERROR_OUT_OF_POOL_MEMORY, "VK_ERROR_OUT_OF_POOL_MEMORY"},
    {VK_ERROR_INVALID_EXTERNAL_HANDLE, "VK_ERROR_INVALID_EXTERNAL_HANDLE"},
    {VK_ERROR_FRAGMENTATION, "VK_ERROR_FRAGMENTATION"},
    {VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS, "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS"},
    {VK_ERROR_SURFACE_LOST_KHR, "VK_ERROR_SURFACE_LOST_KHR"},
    {VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"},
    {VK_SUBOPTIMAL_KHR, "VK_SUBOPTIMAL_KHR"},
    {VK_ERROR_OUT_OF_DATE_KHR, "VK_ERROR_OUT_OF_DATE_KHR"},
    {VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"},
    {VK_ERROR_VALIDATION_FAILED_EXT, "VK_ERROR_VALIDATION_FAILED_EXT"},
    {VK_ERROR_INVALID_SHADER_NV, "VK_ERROR_INVALID_SHADER_NV"},
    {VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT, "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT"},
    {VK_ERROR_NOT_PERMITTED_EXT, "VK_ERROR_NOT_PERMITTED_EXT"},
    {VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT"},
    {VK_THREAD_IDLE_KHR, "VK_THREAD_IDLE_KHR"},
    {VK_THREAD_DONE_KHR, "VK_THREAD_DONE_KHR"},
    {VK_OPERATION_DEFERRED_KHR, "VK_OPERATION_DEFERRED_KHR"},
    {VK_OPERATION_NOT_DEFERRED_KHR, "VK_OPERATION_NOT_DEFERRED_KHR"},
    {VK_PIPELINE_COMPILE_REQUIRED_EXT, "VK_PIPELINE_COMPILE_REQUIRED_EXT"},
    {VK_ERROR_OUT_OF_POOL_MEMORY_KHR, "VK_ERROR_OUT_OF_POOL_MEMORY_KHR"},
    {VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR, "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR"},
    {VK_ERROR_FRAGMENTATION_EXT, "VK_ERROR_FRAGMENTATION_EXT"},
    {VK_ERROR_INVALID_DEVICE_ADDRESS_EXT, "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT"},
};

// Log an info message
#define GFX_INFO(fmt, ...) fprintf(stdout, "INFO: " fmt "\n", ##__VA_ARGS__);

#ifdef NDEBUG
#define GFX_DEBUG(fmt, ...)
#else
// Log a debug message. Only visible if NDEBUG is not defined.
#define GFX_DEBUG(fmt, ...) fprintf(stdout, "\x1B[1;92mDEBUG: \x1B[0m" fmt "\n", ##__VA_ARGS__);
#endif

// Log a warning message
#define GFX_WARNING(fmt, ...)                                                                                          \
    fprintf(stderr, "\x1B[1;93mWARNING: \x1B[0m%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);

#ifdef NDEBUG
// Log an error message
#define GFX_ERROR(fmt, ...)                                                                                            \
    fprintf(stderr, "\x1B[1;91mERROR: \x1B[0m%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);                    \
    abort();
#else
#if GFX_WINDOWS
// Log an error message. Will cause a breakpoint if NDEBUG is not defined.
#define GFX_ERROR(fmt, ...)                                                                                            \
    fprintf(stderr, "\x1B[1;91mERROR: \x1B[0m%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);                    \
    __debugbreak();
#elif GFX_LINUX
// Log an error message. Will cause a SIGTRAP if NDEBUG is not defined.
#define GFX_ERROR(fmt, ...)                                                                                            \
    fprintf(stderr, "\x1B[1;91mERROR: \x1B[0m%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);                    \
    std::raise(SIGTRAP);
#endif
#endif

// Check for Vulkan function call for errors
#define VK_CHECK(x)                                                                                                    \
    do {                                                                                                               \
        VkResult res = x;                                                                                              \
        if (res) {                                                                                                     \
            for (size_t i = 0; i < GFX_ARRAY_LEN(gfxVulkanResults); i++) {                                             \
                if ((res) == gfxVulkanResults[i].result) {                                                             \
                    GFX_ERROR("%s", gfxVulkanResults[i].string);                                                       \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)


// GFX type definitions //

// Properties of the chosen physical device
typedef struct GfxDeviceProperties {
    VkPhysicalDeviceProperties physicalDevice;
    VkPhysicalDeviceMemoryProperties memory;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipeline;
} GfxDeviceProperties;

// Device contains a Vulkan context for rendering. The GFX device is
// monolithic and is setup through gfxCreateInstance() and gfxCreateDevice().
// Release resources with gfxDestroyDevice() and gfxDestroyInstance().
typedef struct GfxDevice {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    GfxDeviceProperties properties;
    VkCommandPool commandPool;
    VkQueue queue;
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    uint32_t queueFamilyIndex;
    uint32_t apiVersion;
    bool vsync;
} GfxDevice;

// Swapchain abstracts the handling of swapchain images and frames in flight.
// The GFX swapchain is monolithic and is setup through
// gfxCreateSwapchain(). Use gfxAcquiteNextImage() and gfxPresent() to acquire and
// present images from the swapchain respectively. The swapchain can be
// recreated with gfxRecreateSwapchain() if the framebuffer size changed. The
// framebufferSizeCallback() function pointer will be used to retrieve the new
// framebuffer size. Release resources with gfxDestroySwapchain().
typedef struct GfxSwapchain {
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;
    bool recreated;

    void (*framebufferSizeCallback)(uint32_t*, uint32_t*);

    struct SupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        uint32_t formatCount;
        VkSurfaceFormatKHR* formats;
        uint32_t presentCount;
        VkPresentModeKHR* presentModes;
    } supportDetails;

    VkImage* images;
    VkImageView* imageViews;
    uint32_t imageCount;
    uint32_t imageIndex;

    VkCommandBuffer* commandBuffers;
    VkSemaphore* renderFinishedSemaphores;
    VkSemaphore* inFlightSemaphores;
    VkFence* inFlightFences;
    uint32_t framesInFlight;
    uint32_t inFlightIndex;
} GfxSwapchain;

// Buffer abstracts a Vulkan buffer and memory allocation. Prefer to use large
// buffers and offsets than many small buffers, that would cause many small
// allocations. Buffers are created with gfxCreateBuffer(). Copy data from host
// to the device allocated buffer with gfxCopyBufferFromHost(). A buffer with
// host coherent memory will always be mapped on pHostMap. When copying buffers
// from host to device with a non-host coherent memory, a staging buffer will be
// used. Realse resources with gfxDestroyDevice().
typedef struct GfxBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
    VkDeviceSize size;
    void* pHostMap;
    VkDescriptorBufferInfo bufferInfo;
} GfxBuffer;

// Image abstracts a Vulkan image, image view and memory allocation. Use
// gfxCreateImage() to create a new image and gfxCreateImageView() to create an
// image view for a created image. Release resources with gfxDestroyImage().
typedef struct GfxImage {
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels;
    VkFormat format;
    VkImageTiling tiling;
} GfxImage;

// Different textures types for creating textures.
enum GfxTextureType {
    GFX_TEXTURE_1D,
    GFX_TEXTURE_2D,
    GFX_TEXTURE_3D,
    GFX_TEXTURE_CUBE_MAP,
    GFX_TEXTURE_TYPE_COUNT,
};

// Texture is a combination of an GFX image and sampler. Create a new
// texture with gfxCreateTexture() and passing it a pointer to the texture data.
// If stb_image.h is available and GFX_USE_STB_IMAGE has been defined,
// textures can be created from files using gfxCreateTextureFromFile(). Mipmaps
// are automatically generated if specified. Use gfxSetTextureSampler() to assign
// a sampler for the texture. The write descriptor for the texture can be
// retrieved with gfxGetTextureDescriptor(). Release resources with
// gfxDestroyTexture().
typedef struct GfxTexture {
    GfxImage image;
    VkDescriptorImageInfo imageInfo;
    VkSampler sampler;
    VkFilter minFilter;
    VkFilter magFilter;
    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
    VkSamplerAddressMode addressModeW;
    VkSamplerMipmapMode mipmapMode;
} GfxTexture;

// Attachment keeps track of color and depth attachments and prepares for
// dynamic rendering. Create a new pass attachment description with
// gfxCreateAttachment(). Before rendering with the attachment call
// gfxBeginRendering() and call gfxEndRendering() when done. If the attachment
// changes (resolution change for instance), it should be recreated.
// gfxRecreateAttachment() can be used, or alternatively gfxDestroyAttachment()
// and gfxCreateAttachment(). Release resources with gfxDestroyAttachment().
typedef struct GfxAttachment {
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo;
    VkRenderingAttachmentInfo* pRenderingAttachmentInfos;
    VkFormat* pFormats;
    uint32_t colorAttachmentCount;
    GfxImage* pColorAttachments;
    GfxImage* pDepthAttachment;
    GfxImage* pResolveAttachment;
} GfxAttachment;

// Layout abstracts the use of descriptor set layouts and pipeline layouts.
// Create a new layout with gfxCreateLayout(). Only one descriptor set is
// supported and it is a push descriptor. Release resources with
// gfxDestroyLayout().
typedef struct GfxLayout {
    VkDescriptorSetLayout setLayout;
    uint32_t pushConstantRangeCount;
    VkPushConstantRange* pPushConstantRanges;
    VkPipelineLayout pipelineLayout;
} GfxLayout;

// Shader abstracts the handling of shaders and builds ontop of Vulkans shader
// objects. Create a new shader from SPIR-V code with gfxCreateShader(). To load
// a shader from GLSL source code, use gfxCreateShaderFromFileGLSL(). Before
// using the shader, the shader has to be build. Either use gfxBuildShader(), or
// use gfxBuildLinkedShaders() to build an optimized vertex-fragment shader pair.
// For rendering, the active shader has to be bound: use gfxCmdBindShader().
// Release resources with gfxDestroyShader().
typedef struct GfxShader {
    VkShaderEXT shader;
    VkShaderCreateInfoEXT createInfo;
    char* pPath;
    void* pCode;
} GfxShader;


// Monolithic global variables //

extern GfxDevice gfxDevice;
extern GfxSwapchain gfxSwapchain;


// Function declarations //

/// <summary>
/// Create a new instance. Handle is internally managed and accessible through
/// gfxDevice.
/// </summary>
/// <param name="apiVersion">A Vulkan API version, VK_API_VERSION_* or VK_MAKE_API_VERSION()</param>
/// <param name="instanceExtensionCount">Number of instance extensions</param>
/// <param name="ppInstanceExtensions">List of instance extensions to use</param>
void gfxCreateInstance(uint32_t apiVersion, uint32_t instanceExtensionCount, const char** ppInstanceExtensions);

/// <summary>
/// Release resources for the instance.
/// </summary>
void gfxDestroyInstance();

/// <summary>
/// Create a new device. Handle is internally managed and accessible through
/// gfxDevice.
/// </summary>
/// <param name="physicalDeviceIndex">Physical device index to use</param>
/// <param name="deviceExtensionCount">Number of device extensions</param>
/// <param name="ppDeviceExtensions">List of device extensions to use</param>
/// <param name="features">Pointer to features that will be put in pNext of VkDeviceCreateInfo, can be NULL</param>
/// <param name="surface">Surface to use</param>
void gfxCreateDevice(uint32_t physicalDeviceIndex, uint32_t deviceExtensionCount, const char** ppDeviceExtensions,
                     VkPhysicalDeviceFeatures2* features, VkSurfaceKHR surface);

/// <summary>
/// Release resources for the device.
/// </summary>
void gfxDestroyDevice();

/// <summary>
/// Get the sample count of the current device.
/// </summary>
/// <returns>Sample count</returns>
VkSampleCountFlagBits gfxGetDeviceSampleCount();

/// <summary>
/// Create a new swapchain. Handle is internally managed and accessible through
/// gfxSwapchain.
/// </summary>
/// <param name="framesInFlight">Number of frames in flight to use</param>
/// <param name="framebufferSizeCallback">Callback function where the current framebuffer size can be retrieved</param>
void gfxCreateSwapchain(uint32_t framesInFlight, void (*framebufferSizeCallback)(uint32_t*, uint32_t*));

/// <summary>
/// Release the resources for the swapchain.
/// </summary>
void gfxDestroySwapchain();

/// <summary>
/// Recreate the swapchain, for instance if the framebuffer size changed. New
/// framebuffer size is retrieved through framebufferSizeCallback().
/// </summary>
void gfxRecreateSwapchain();

/// <summary>
/// Acquire a new image from the swapchain. This call will block until an image
/// is available.
/// </summary>
/// <returns>Command buffer to use to render to the new image</returns>
VkCommandBuffer gfxAcquireNextImage();

/// <summary>
/// Present an image to the swapchain by blitting it.
/// </summary>
/// <param name="cmd">Command buffer retrieved from a call to
/// gfxAcquireNextImage()</param> <param name="pImage"></param>
void gfxPresent(VkCommandBuffer cmd, GfxImage* pImage);

/// <summary>
/// Create a new buffer.
/// </summary>
/// <param name="size">Size of buffer in bytes</param>
/// <param name="usage">How the buffer will be used</param>
/// <param name="properties">Properties of the memory to allocate</param>
/// <param name="pBuffer">Where the created buffer will be stored</param>
void gfxCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, GfxBuffer* pBuffer);

/// <summary>
/// Release resources for a buffer.
/// </summary>
/// <param name="pBuffer">Buffer to destroy</param>
void gfxDestroyBuffer(GfxBuffer* pBuffer);

/// <summary>
/// Copy data from host memory to a buffer's device memory. If buffer's memory
/// is not host coherent, a staging buffer will be used.
/// </summary>
/// <param name="pBuffer">Buffer to use</param>
/// <param name="pData">Pointer to data to copy</param>
/// <param name="size">Size of data to copy in bytes</param>
/// <param name="offset">Offset into buffer's device memory to put the data</param>
void gfxCopyBufferFromHost(const GfxBuffer* pBuffer, const void* pData, VkDeviceSize size, VkDeviceSize offset);

/// <summary>
/// Get the write descriptor of a buffer.
/// </summary>
/// <param name="pBuffer">Buffer to use</param>
/// <param name="binding">In which binding the buffer descriptor should be placed</param>
/// <param name="type">Type of descriptor</param>
/// <param name="offset">Offset into buffer to bind</param>
/// <param name="range">Range in bytes to bind, can be VK_WHOLE_SIZE</param>
/// <returns>A write descriptor set</returns>
VkWriteDescriptorSet gfxGetBufferDescriptor(GfxBuffer* pBuffer, uint32_t binding, VkDescriptorType type,
                                            VkDeviceSize offset, VkDeviceSize range);

/// <summary>
/// Create a new image.
/// </summary>
/// <param name="width">Width of image</param>
/// <param name="height">Height of image</param>
/// <param name="mipLevels">Mipmap levels to use</param>
/// <param name="samples">Sample count</param>
/// <param name="format">Format to use</param>
/// <param name="tiling">Tiling to use</param>
/// <param name="usage">How the image will be used</param>
/// <param name="properties">Properties of the memory to allocate</param>
/// <param name="pImage">Where the created image will be stored</param>
void gfxCreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format,
                    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, GfxImage* pImage);

/// <summary>
/// Release resources for an image.
/// </summary>
/// <param name="pImage">Image to destroy</param>
void gfxDestroyImage(GfxImage* pImage);

/// <summary>
/// Create an image view for an image.
/// </summary>
/// <param name="pImage">Image to use</param>
/// <param name="aspectFlags">Aspect flags to use for image view</param>
void gfxCreateImageView(GfxImage* pImage, VkImageAspectFlags aspectFlags);

/// <summary>
/// Create a new texture.
/// </summary>
/// <param name="type">Type of texture to create</param>
/// <param name="format">Format to use</param>
/// <param name="pData">Pointer to data with texture pixel values</param>
/// <param name="width">Width of texture</param>
/// <param name="height">Height of texture</param>
/// <param name="channels">Number of channels in texture</param>
/// <param name="generateMipmaps">Whether to generate mipmaps or not</param>
/// <param name="pTexture">Where the created texture will be stored</param>
void gfxCreateTexture(enum GfxTextureType type, VkFormat format, const void* pData, uint32_t width, uint32_t height,
                      uint32_t channels, bool generateMipmaps, GfxTexture* pTexture);

#ifdef GFX_USE_STB_IMAGE
/// <summary>
/// Create a new texture from file.
/// </summary>
/// <param name="type">Type of texture to create</param>
/// <param name="format">Format to use</param>
/// <param name="pPath">Path to texture file</param>
/// <param name="generateMipmaps">Whether to generate mipmaps or not</param>
/// <param name="pTexture">Where the created texture will be stored</param>
void gfxCreateTextureFromFile(enum GfxTextureType type, VkFormat format, const char* pPath, bool generateMipmaps,
                              GfxTexture* pTexture);
#endif

/// <summary>
/// Release resources for a texture.
/// </summary>
/// <param name="pTexture">Texture to destroy</param>
void gfxDestroyTexture(GfxTexture* pTexture);

/// <summary>
/// Get the write descriptor of a texture.
/// </summary>
/// <param name="pTexture">Texture to use</param>
/// <param name="binding">In which binding the texture descriptor should be placed</param>
/// <param name="type">Type of descriptor</param>
/// <returns>A write descriptor set</returns>
VkWriteDescriptorSet gfxGetTextureDescriptor(const GfxTexture* pTexture, uint32_t binding, VkDescriptorType type);

/// <summary>
/// Set texture magnification filter.
/// </summary>
/// <param name="pTexture">Texture to use</param>
/// <param name="magFilter">Magnification filter</param>
void gfxSetTextureMagFilter(GfxTexture* pTexture, VkFilter magFilter);

/// <summary>
/// Set texture minification filter.
/// </summary>
/// <param name="pTexture">Texture to use</param>
/// <param name="minFilter">Minification filter</param>
void gfxSetTextureMinFilter(GfxTexture* pTexture, VkFilter minFilter);

/// <summary>
/// Set texture mipmap mode.
/// </summary>
/// <param name="pTexture">Texture to use</param>
/// <param name="mipmapMode">Mipmap mode</param>
void gfxSetTextureMipmapMode(GfxTexture* pTexture, VkSamplerMipmapMode mipmapMode);

/// <summary>
/// Set texture address mode.
/// </summary>
/// <param name="pTexture">Texture to use</param>
/// <param name="modeU">Address mode U</param>
/// <param name="modeV">Address mode V</param>
/// <param name="modeW">Address mode W</param>
void gfxSetTextureAddressMode(GfxTexture* pTexture, VkSamplerAddressMode modeU, VkSamplerAddressMode modeV,
                              VkSamplerAddressMode modeW);

/// <summary>
/// Create a new pass.
/// </summary>
/// <param name="colorAttachmentCount">Number of color attachments</param>
/// <param name="pColorAttachments">List of images that should be used as color attachments</param>
/// <param name="pDepthAttachment">Depth attachment to use, can be NULL</param>
/// <param name="pResolveAttachment">Resolve attachment to use, can be NULL</param>
/// <param name="pAttachment">Where the created pass will be stored</param>
void gfxCreateAttachment(uint32_t colorAttachmentCount, GfxImage* pColorAttachments, GfxImage* pDepthAttachment,
                         GfxImage* pResolveAttachment, GfxAttachment* pAttachment);

/// <summary>
/// Release resource for a pass.
/// </summary>
/// <param name="pAttachment">Pass to destroy</param>
void gfxDestroyAttachment(GfxAttachment* pAttachment);

/// <summary>
/// Recreate a pass if attachments changed. Same as calling gfxDestroyAttachment()
/// followed by gfxCreateAttachment().
/// </summary>
/// <param name="pAttachment">Pass to recreate</param>
/// <param name="colorAttachmentCount">Number of color attachments</param>
/// <param name="pColorAttachments">List of images that should be used as color attachments</param>
/// <param name="pDepthAttachment">Depth attachment to use, can be NULL</param>
/// <param name="pResolveAttachment">Resolve attachment to use, can be NULL</param>
void gfxRecreateAttachment(GfxAttachment* pAttachment, uint32_t colorAttachmentCount, GfxImage* pColorAttachments,
                           GfxImage* pDepthAttachment, GfxImage* pResolveAttachment);

/// <summary>
/// Begin dynamic rendering using a pass.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pAttachment">Pass to use</param>
/// <param name="clearValue">Clear value for attachments</param>
/// <param name="loadOp">Load operation for attachments</param>
void gfxCmdBeginPass(VkCommandBuffer cmd, const GfxAttachment* pAttachment, VkClearValue clearValue,
                     VkAttachmentLoadOp loadOp);

/// <summary>
/// End dynamic rendering using a pass.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pAttachment">Pass to use</param>
void gfxCmdEndPass(VkCommandBuffer cmd, const GfxAttachment* pAttachment);

/// <summary>
/// Create a new layout.
/// </summary>
/// <param name="bindingCount">Number of bindings</param>
/// <param name="pTypes">List of descriptor types, one per binding</param>
/// <param name="pStages">List of shader stages, one per binding</param>
/// <param name="pCounts">List of descriptor counts, one per binding</param>
/// <param name="pushConstantRangeCount">Number of push constant ranges</param>
/// <param name="pPushConstantRanges">List of push constant ranges</param>
/// <param name="pLayout">Where the created layout will be stored</param>
void gfxCreateLayout(uint32_t bindingCount, VkDescriptorType* pTypes, VkShaderStageFlags* pStages, uint32_t* pCounts,
                     uint32_t pushConstantRangeCount, VkPushConstantRange* pPushConstantRanges, GfxLayout* pLayout);

/// <summary>
/// Release resources for a layout.
/// </summary>
/// <param name="pLayout">Layout to destroy</param>
void gfxDestroyLayout(GfxLayout* pLayout);

/// <summary>
/// Create a new shader from SPIR-V byte code.
/// </summary>
/// <param name="pCode">Pointer to SPIR-V byte code</param>
/// <param name="codeSize">Size of code in bytes</param>
/// <param name="stage">Which stage the shader represents</param>
/// <param name="nextStage">Which stages can come after this shader</param>
/// <param name="pLayout">Layout to use</param>
/// <param name="pShader">Where the created shader will be stored</param>
void gfxCreateShader(const void* pCode, size_t codeSize, VkShaderStageFlagBits stage, VkShaderStageFlags nextStage,
                     const GfxLayout* pLayout, GfxShader* pShader);

/// <summary>
/// Create a new shader from GLSL source code.
/// </summary>
/// <param name="pPath">Path to GLSL source code</param>
/// <param name="stage">Which stage the shader represents</param>
/// <param name="nextStage">Which stages can come after this shader</param>
/// <param name="pLayout">Layout to use</param>
/// <param name="pShader">Where the created shader will be stored</param>
void gfxCreateShaderFromFileGLSL(const char* pPath, VkShaderStageFlagBits stage, VkShaderStageFlags nextStage,
                                 const GfxLayout* pLayout, GfxShader* pShader);

/// <summary>
/// Release resources for a shader.
/// </summary>
/// <param name="pShader">Shader to destroy</param>
void gfxDestroyShader(GfxShader* pShader);

/// <summary>
/// Build a shader.
/// </summary>
/// <param name="pShader">Shader to build</param>
void gfxBuildShader(GfxShader* pShader);

/// <summary>
/// Build a linked vertex and fragment shader.
/// </summary>
/// <param name="pVertexShader">Vertex shader to build</param>
/// <param name="pFragmentShader">Fragment shader to build</param>
void gfxBuildLinkedShaders(GfxShader* pVertexShader, GfxShader* pFragmentShader);

/// <summary>
/// Bind a shader for rendering.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pShader">Shader to bind</param>
void gfxCmdBindShader(VkCommandBuffer cmd, const GfxShader* pShader);

/// <summary>
/// Set default states for rendering using shader objects.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="vertexBindingDescriptionCount">Number of vertex binding descriptions</param>
/// <param name="vertexBindingDescriptions">List of vertex binding descriptions</param>
/// <param name="vertexAttributeDescriptionCount">Number of vertex attribute descriptions</param>
/// <param name="vertexAttributeDescriptions">List of vertex attrubyte descriptions</param>
void gfxCmdSetDefaultStates(VkCommandBuffer cmd, uint32_t vertexBindingDescriptionCount,
                            const VkVertexInputBindingDescription2EXT* vertexBindingDescriptions,
                            uint32_t vertexAttributeDescriptionCount,
                            const VkVertexInputAttributeDescription2EXT* vertexAttributeDescriptions);

// Inlined helper functions //

/// <summary>
/// Call malloc and check for valid pointer.
/// </summary>
/// <param name="sz">Size in bytes to allocate</param>
/// <returns>Pointer to allocated memory</returns>
inline void* gfxCheckedMalloc(size_t sz)
{
    void* p = malloc(sz);
    if (!p) {
        GFX_ERROR("Unable to allocate memory");
    }
    return p;
}

/// <summary>
/// Align value to alignment.
/// </summary>
/// <param name="value">Value to align</param>
/// <param name="alignment">Required alignment</param>
/// <returns></returns>
inline VkDeviceSize gfxAlignTo(VkDeviceSize value, VkDeviceSize alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

/// <summary>
/// Create and begin a new one-time-use command buffer.
/// </summary>
/// <returns>A new command buffer</returns>
inline VkCommandBuffer gfxCmdBegin()
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    VkCommandBufferAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = gfxDevice.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    VK_CHECK(vkAllocateCommandBuffers(gfxDevice.device, &ai, &cmd));

    VkCommandBufferBeginInfo bi = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VK_CHECK(vkBeginCommandBuffer(cmd, &bi));

    return cmd;
}

/// <summary>
/// Submit and destroy a one-time-use command buffer.
/// </summary>
/// <param name="cmd">Command buffer to submit and destroy</param>
inline void gfxCmdEnd(VkCommandBuffer cmd)
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo si = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    VK_CHECK(vkQueueSubmit(gfxDevice.queue, 1, &si, NULL));
    VK_CHECK(vkQueueWaitIdle(gfxDevice.queue));

    vkFreeCommandBuffers(gfxDevice.device, gfxDevice.commandPool, 1, &cmd);
}

/// <summary>
/// Find a memory type given a type filter and required memory properties.
/// </summary>
/// <param name="typeFilter">Type filter to use</param>
/// <param name="properties">Memory property flags</param>
/// <returns>Index to memory type</returns>
inline uint32_t gfxFindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    for (uint32_t i = 0; i < gfxDevice.properties.memory.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (gfxDevice.properties.memory.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    GFX_ERROR("Failed to find suitable memory type");
    return UINT32_MAX;
}

/// <summary>
/// Find a supported format from the current device, given a set of candidate
/// formats.
/// </summary>
/// <param name="pCandidates">List of candidate formats</param>
/// <param name="candidateCount">Number of candidates</param>
/// <param name="tiling">Required image tiling</param>
/// <param name="features"Format feature flags></param>
/// <returns>A supported format</returns>
inline VkFormat gfxFindSupportedFormat(VkFormat* pCandidates, uint32_t candidateCount, VkImageTiling tiling,
                                       VkFormatFeatureFlags features)
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    for (uint32_t i = 0; i < candidateCount; i++) {
        VkFormat c = pCandidates[i];

        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(gfxDevice.physicalDevice, c, &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return c;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return c;
        }
    }
    return VK_FORMAT_UNDEFINED;
}

/// <summary>
/// Find a depth format supported by the current device.
/// </summary>
/// <returns>A supported depth format</returns>
inline VkFormat gfxFindDepthFormat()
{
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };

    return gfxFindSupportedFormat(candidates, GFX_ARRAY_LEN(candidates), VK_IMAGE_TILING_OPTIMAL,
                                  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

/// <summary>
/// Create an image barrier.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="srcStage">Stage to wait for</param>
/// <param name="srcAccess">Type of access to wait for</param>
/// <param name="dstStage">Stage before which the barrier has to take place</param>
/// <param name="dstAccess">Type of acces before which the barrer has to take place</param>
/// <param name="oldLayout">Previous image layout</param>
/// <param name="newLayout">New image layout</param>
/// <param name="image">Image to use</param>
/// <param name="pSubresourceRange">Subresource range to use, can be NULL in which case a default subrange is
/// used.</param>
inline void gfxImageBarrier(VkCommandBuffer cmd, VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                            VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess, VkImageLayout oldLayout,
                            VkImageLayout newLayout, VkImage image, VkImageSubresourceRange* pSubresourceRange)
{
    VkImageSubresourceRange defaultSubresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = srcStage,
        .srcAccessMask = srcAccess,
        .dstStageMask = dstStage,
        .dstAccessMask = dstAccess,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = pSubresourceRange ? *pSubresourceRange : defaultSubresourceRange,
    };

    VkDependencyInfo dependencyInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };

    vkCmdPipelineBarrier2(cmd, &dependencyInfo);
}

/// <summary>
/// Transition the image layout of a color attachment for rendering. Previous
/// layout is assumed to be VK_IMAGE_LAYOUT_UNDEFINED.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pImage">Image to transition</param>
inline void gfxTransitionForColorAttachment(VkCommandBuffer cmd, GfxImage* pImage)
{
    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, pImage->image, NULL);
}

/// <summary>
/// Transition the image layout of a color attachment for blitting. Previous
/// layout is assumed to be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pImage">Image to transition</param>
inline void gfxTransitionForBlitting(VkCommandBuffer cmd, GfxImage* pImage)
{
    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pImage->image,
                    NULL);
}

// Define GFX_IMPLEMENTATION in exactly one compilation unit before
// including gfxgdrasil.h
#ifdef GFX_IMPLEMENTATION


// Monolithic global variables //

static GfxDevice gfxDevice;
static GfxSwapchain gfxSwapchain;


// Helper macros //

// Reset the memory of an object after releasing its resources.
#define GFX_RESET(x) memset((x), 0, sizeof(*(x)))

// Get the max of two values
#define GFX_MAX(x, y) ((x) > (y) ? (x) : (y))
// Get the min of two values
#define GFX_MIN(x, y) ((x) < (y) ? (x) : (y))
// Clamp a value to an interval
#define GFX_CLAMP(x, low, high) (GFX_MIN(GFX_MAX((x), (low)), (high)))

// Explicitly mark a variable as unused
#define GFX_UNUSED(x) (void)(x)

// Macro for loading a device function pointers as Xvk...()
#define VK_LOAD(func_name)                                                                                             \
    PFN_##func_name X##func_name = (PFN_##func_name)vkGetDeviceProcAddr(gfxDevice.device, #func_name)

// Macro for calling a function via its vkGetDeviceProcAddr name
#define VK_CALL(func_name, ...)                                                                                        \
    do {                                                                                                               \
        PFN_##func_name pfn_##func_name = (PFN_##func_name)vkGetDeviceProcAddr(gfxDevice.device, #func_name);          \
        pfn_##func_name(__VA_ARGS__);                                                                                  \
    } while (0);

#ifndef NDEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData)
{
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        GFX_WARNING("%s: %s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        GFX_ERROR("%s: %s", pCallbackData->pMessageIdName, pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}

static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator,
                                             VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                          const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) {
        func(instance, debugMessenger, pAllocator);
    }
}

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* ci)
{
    *ci = (VkDebugUtilsMessengerCreateInfoEXT){
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };
}

static void createDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT ci;
    populateDebugMessengerCreateInfo(&ci);

    VK_CHECK(createDebugUtilsMessengerEXT(gfxDevice.instance, &ci, NULL, &gfxDevice.debugMessenger));
}
#endif

void gfxCreateInstance(uint32_t apiVersion, uint32_t instanceExtensionCount, const char** ppInstanceExtensions)
{
    gfxDevice.apiVersion = apiVersion;

    VkApplicationInfo ai = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pEngineName = "GFX",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .apiVersion = apiVersion,
    };

    VkInstanceCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &ai,
        .enabledExtensionCount = instanceExtensionCount,
        .ppEnabledExtensionNames = ppInstanceExtensions,
    };

#ifndef NDEBUG
    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
    ci.enabledLayerCount = GFX_ARRAY_LEN(layers);
    ci.ppEnabledLayerNames = layers;

    const char** ppExpandedExtensions = GFX_MALLOC((instanceExtensionCount + 1) * sizeof *ppExpandedExtensions);
    memcpy(ppExpandedExtensions, ppInstanceExtensions, instanceExtensionCount * sizeof *ppExpandedExtensions);
    ppExpandedExtensions[instanceExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    ci.enabledExtensionCount = instanceExtensionCount + 1;
    ci.ppEnabledExtensionNames = ppExpandedExtensions;
#endif

    VK_CHECK(vkCreateInstance(&ci, NULL, &gfxDevice.instance));

    uint32_t version;
    VK_CHECK(vkEnumerateInstanceVersion(&version));
    GFX_INFO("Created Vulkan instance: %d.%d.%d", VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version),
             VK_API_VERSION_PATCH(version));

#ifndef NDEBUG
    createDebugMessenger();
    GFX_FREE(ppExpandedExtensions);
#endif
}

void gfxDestroyInstance()
{
    if (gfxDevice.instance) {
#if defined(_DEBUG)
        destroyDebugUtilsMessengerEXT(gfxDevice.instance, gfxDevice.debugMessenger, NULL);
#endif
        vkDestroyInstance(gfxDevice.instance, NULL);
    }
}

static bool checkDeviceExtensionSupport(uint32_t deviceExtensionCount, const char** ppDeviceExtensions)
{
    uint32_t n;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(gfxDevice.physicalDevice, NULL, &n, NULL));
    VkExtensionProperties* pAvailable = GFX_MALLOC(n * sizeof *pAvailable);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(gfxDevice.physicalDevice, NULL, &n, pAvailable));

    GFX_DEBUG("Requesting device extensions (%d):", deviceExtensionCount);
    for (uint32_t i = 0; i < deviceExtensionCount; i++) {
        GFX_DEBUG(" * %s", ppDeviceExtensions[i]);
    }

    GFX_DEBUG("Available device extensions (%d):", n);
    for (uint32_t i = 0; i < n; i++) {
        GFX_DEBUG(" * %s", pAvailable[i].extensionName);
    }

    bool result = true;
    for (uint32_t i = 0; i < deviceExtensionCount; i++) {
        bool found = false;
        for (uint32_t j = 0; j < n; j++) {
            if (!strcmp(ppDeviceExtensions[i], pAvailable[j].extensionName)) {
                found = true;
                break;
            }
        }

        if (!found) {
            GFX_ERROR("The requested extension %s is not available", ppDeviceExtensions[i]);
            result = false;
        }
    }

    GFX_FREE(pAvailable);

    return result;
}

static uint32_t getQueueFamilyIndex(VkSurfaceKHR surface, uint32_t requiredFamilyFlags)
{
    uint32_t n;
    vkGetPhysicalDeviceQueueFamilyProperties(gfxDevice.physicalDevice, &n, NULL);
    VkQueueFamilyProperties* pProps = GFX_MALLOC(n * sizeof *pProps);
    vkGetPhysicalDeviceQueueFamilyProperties(gfxDevice.physicalDevice, &n, pProps);

    if (!n) {
        GFX_ERROR("No Vulkan queue family available");
    }

    struct VulkanQueue {
        VkQueueFlagBits flagBit;
        const char* string;
    } vulkanQueues[] = {
        {VK_QUEUE_GRAPHICS_BIT, "VK_QUEUE_GRAPHICS_BIT"},
        {VK_QUEUE_COMPUTE_BIT, "VK_QUEUE_COMPUTE_BIT"},
        {VK_QUEUE_TRANSFER_BIT, "VK_QUEUE_TRANSFER_BIT"},
        {VK_QUEUE_SPARSE_BINDING_BIT, "VK_QUEUE_SPARSE_BINDING_BIT"},
        {VK_QUEUE_PROTECTED_BIT, "VK_QUEUE_PROTECTED_BIT"},
        {VK_QUEUE_VIDEO_DECODE_BIT_KHR, "VK_QUEUE_VIDEO_DECODE_BIT_KHR"},
        {VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VK_QUEUE_VIDEO_ENCODE_BIT_KHR"},
        {VK_QUEUE_OPTICAL_FLOW_BIT_NV, "VK_QUEUE_OPTICAL_FLOW_BIT_NV"},
    };

    GFX_DEBUG("Available queue families for selected device (%d):", n);
    for (uint32_t i = 0; i < n; i++) {
        GFX_DEBUG(" * [%d]:", i);
        for (uint32_t j = 0; j < GFX_ARRAY_LEN(vulkanQueues); j++) {
            if (pProps[i].queueFlags & vulkanQueues[j].flagBit) {
                GFX_DEBUG("         %s", vulkanQueues[j].string);
            }
        }
    }

    uint32_t index = 0;
    for (uint32_t i = 0; i < n; i++) {
        if (pProps[i].queueFlags & requiredFamilyFlags) {
            break;
        }
        index++;
    }

    if (index == n) {
        GFX_ERROR("No Vulkan queue found for requested families");
    }

    // Check that the selected queue family supports PRESENT
    VkBool32 supported;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(gfxDevice.physicalDevice, index, surface, &supported));
    if (!supported) {
        GFX_ERROR("Selected queue family does not support PRESENT");
    }

    GFX_FREE(pProps);

    return index;
}

void gfxCreateDevice(uint32_t physicalDeviceIndex, uint32_t deviceExtensionCount, const char** ppDeviceExtensions,
                     VkPhysicalDeviceFeatures2* features, VkSurfaceKHR surface)
{
    if (!gfxDevice.instance) {
        GFX_ERROR("Instance not initialized");
    }

    if (gfxDevice.device) {
        GFX_ERROR("Device already created");
    }

    gfxDevice.surface = surface;

    // Iterate all physical devices
    uint32_t n;
    VK_CHECK(vkEnumeratePhysicalDevices(gfxDevice.instance, &n, NULL));
    VkPhysicalDevice* pPhysicalDevices = GFX_MALLOC(n * sizeof *pPhysicalDevices);
    VK_CHECK(vkEnumeratePhysicalDevices(gfxDevice.instance, &n, pPhysicalDevices));

    GFX_INFO("Available devices (%d):", n);
    for (uint32_t i = 0; i < n; i++) {
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtp = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR,
        };

        VkPhysicalDeviceDriverProperties driver = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
            .pNext = &rtp,
        };

        VkPhysicalDeviceProperties2 prop = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &driver,
        };

        vkGetPhysicalDeviceProperties2(pPhysicalDevices[i], &prop);

        if (i == physicalDeviceIndex) {
            gfxDevice.physicalDevice = pPhysicalDevices[i];
            vkGetPhysicalDeviceProperties(gfxDevice.physicalDevice, &gfxDevice.properties.physicalDevice);
            vkGetPhysicalDeviceMemoryProperties(gfxDevice.physicalDevice, &gfxDevice.properties.memory);
            gfxDevice.properties.rayTracingPipeline = rtp;
        }

        GFX_INFO(" * [%d] %s, driver: %s %s, Vulkan %d.%d.%d %s", i, prop.properties.deviceName, driver.driverName,
                 driver.driverInfo, VK_API_VERSION_MAJOR(prop.properties.apiVersion),
                 VK_API_VERSION_MINOR(prop.properties.apiVersion), VK_API_VERSION_PATCH(prop.properties.apiVersion),
                 i == physicalDeviceIndex ? "(chosen)" : "");
    }

    GFX_FREE(pPhysicalDevices);

    checkDeviceExtensionSupport(deviceExtensionCount, ppDeviceExtensions);

    uint32_t queueFamilyIndex =
        getQueueFamilyIndex(surface, VK_QUEUE_GRAPHICS_BIT && VK_QUEUE_COMPUTE_BIT && VK_QUEUE_TRANSFER_BIT);

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    VkDeviceCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = deviceExtensionCount,
        .ppEnabledExtensionNames = ppDeviceExtensions,
    };

    VK_CHECK(vkCreateDevice(gfxDevice.physicalDevice, &ci, NULL, &gfxDevice.device));

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndex,
    };

    VK_CHECK(vkCreateCommandPool(gfxDevice.device, &commandPoolCreateInfo, NULL, &gfxDevice.commandPool));
    vkGetDeviceQueue(gfxDevice.device, queueFamilyIndex, 0, &gfxDevice.queue);
}

void gfxDestroyDevice()
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    vkDeviceWaitIdle(gfxDevice.device);

    if (gfxDevice.commandPool) {
        vkDestroyCommandPool(gfxDevice.device, gfxDevice.commandPool, NULL);
    }
    if (gfxDevice.device) {
        vkDestroyDevice(gfxDevice.device, NULL);
    }
    if (gfxDevice.surface) {
        vkDestroySurfaceKHR(gfxDevice.instance, gfxDevice.surface, NULL);
    }

    GFX_RESET(&gfxDevice);
}

VkSampleCountFlagBits gfxGetDeviceSampleCount()
{
    VkSampleCountFlags counts = gfxDevice.properties.physicalDevice.limits.framebufferColorSampleCounts &
                                gfxDevice.properties.physicalDevice.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT) {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT) {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT) {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT) {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT) {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

static void querySupport()
{
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfxDevice.physicalDevice, gfxDevice.surface,
                                                       &gfxSwapchain.supportDetails.capabilities));

    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gfxDevice.physicalDevice, gfxDevice.surface,
                                                  &gfxSwapchain.supportDetails.formatCount, NULL));
    gfxSwapchain.supportDetails.formats =
        GFX_MALLOC(gfxSwapchain.supportDetails.formatCount * sizeof *gfxSwapchain.supportDetails.formats);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gfxDevice.physicalDevice, gfxDevice.surface,
                                                  &gfxSwapchain.supportDetails.formatCount,
                                                  gfxSwapchain.supportDetails.formats));

    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gfxDevice.physicalDevice, gfxDevice.surface,
                                                       &gfxSwapchain.supportDetails.presentCount, NULL));
    gfxSwapchain.supportDetails.presentModes =
        GFX_MALLOC(gfxSwapchain.supportDetails.presentCount * sizeof *&gfxSwapchain.supportDetails.presentModes);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gfxDevice.physicalDevice, gfxDevice.surface,
                                                       &gfxSwapchain.supportDetails.presentCount,
                                                       gfxSwapchain.supportDetails.presentModes));
}

// Iterate through available formats and return the one that matches what we
// want. If what we want is not available, the first format in the array is
// returned.
static VkSurfaceFormatKHR chooseSurfaceFormat(uint32_t availableFormatsCount,
                                              const VkSurfaceFormatKHR* availableFormats)
{
    for (uint32_t i = 0; i < availableFormatsCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }
    return availableFormats[0];
}

// Iterate through available present modes and see if a no v-sync mode is
// available, otherwise go for VK_PRESENT_MODE_FIFO_KHR. If vsync is requested,
// just go for VK_PRESENT_MODE_FIFO_KHR.
static VkPresentModeKHR choosePresentMode(uint32_t availablePresentModeCount,
                                          const VkPresentModeKHR* availablePresentModes, bool vsync)
{
    if (vsync) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    for (uint32_t i = 0; i < availablePresentModeCount; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

// Set the resolution of the swapchain images. Use the size of the framebuffer
// from the window.
static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR* capabilities, uint32_t width, uint32_t height)
{
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        VkExtent2D actualExtent = {
            .width = (uint32_t)width,
            .height = (uint32_t)height,
        };

        actualExtent.width =
            GFX_CLAMP(actualExtent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
        actualExtent.height =
            GFX_CLAMP(actualExtent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

        return actualExtent;
    }
}

static void createSwapchain()
{
    uint32_t width;
    uint32_t height;
    gfxSwapchain.framebufferSizeCallback(&width, &height);

    VkSurfaceFormatKHR surfaceFormat =
        chooseSurfaceFormat(gfxSwapchain.supportDetails.formatCount, gfxSwapchain.supportDetails.formats);
    VkPresentModeKHR presentMode = choosePresentMode(gfxSwapchain.supportDetails.presentCount,
                                                     gfxSwapchain.supportDetails.presentModes, gfxDevice.vsync);
    VkExtent2D extent = chooseExtent(&gfxSwapchain.supportDetails.capabilities, width, height);

    // Using at least minImageCount number of images is required but using one
    // extra can avoid unnecessary waits on the driver
    gfxSwapchain.imageCount = gfxSwapchain.supportDetails.capabilities.minImageCount + 1;

    // Also make sure that we are not exceeding the maximum number of images
    if (gfxSwapchain.supportDetails.capabilities.maxImageCount > 0 &&
        gfxSwapchain.imageCount > gfxSwapchain.supportDetails.capabilities.maxImageCount) {
        gfxSwapchain.imageCount = gfxSwapchain.supportDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = gfxDevice.surface,
        .minImageCount = gfxSwapchain.imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1, // Unless rendering stereoscopically
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform = gfxSwapchain.supportDetails.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    gfxSwapchain.format = surfaceFormat.format;
    gfxSwapchain.extent = extent;

    VK_CHECK(vkCreateSwapchainKHR(gfxDevice.device, &ci, NULL, &gfxSwapchain.swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(gfxDevice.device, gfxSwapchain.swapchain, &gfxSwapchain.imageCount, NULL));
    gfxSwapchain.images = GFX_MALLOC(gfxSwapchain.imageCount * sizeof *gfxSwapchain.images);
    gfxSwapchain.imageViews = GFX_MALLOC(gfxSwapchain.imageCount * sizeof *gfxSwapchain.imageViews);
    VK_CHECK(vkGetSwapchainImagesKHR(gfxDevice.device, gfxSwapchain.swapchain, &gfxSwapchain.imageCount,
                                     gfxSwapchain.images));

    for (uint32_t i = 0; i < gfxSwapchain.imageCount; i++) {
        VkImageViewCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = gfxSwapchain.images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = gfxSwapchain.format,
            .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                           .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                           .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                           .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1},
        };

        VK_CHECK(vkCreateImageView(gfxDevice.device, &ci, NULL, &gfxSwapchain.imageViews[i]));
    }
}

static void destroySwapchain()
{
    vkDeviceWaitIdle(gfxDevice.device);

    // Swapchain images are destroyed in vkDestroySwapchainKHR()
    for (uint32_t i = 0; i < gfxSwapchain.imageCount; i++) {
        vkDestroyImageView(gfxDevice.device, gfxSwapchain.imageViews[i], NULL);
    }

    vkDestroySwapchainKHR(gfxDevice.device, gfxSwapchain.swapchain, NULL);

    GFX_FREE(gfxSwapchain.images);
    GFX_FREE(gfxSwapchain.imageViews);
}

static void createSyncObjects()
{
    VkCommandBufferAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = gfxDevice.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = gfxSwapchain.framesInFlight,
    };

    gfxSwapchain.renderFinishedSemaphores =
        GFX_MALLOC(gfxSwapchain.imageCount * sizeof *gfxSwapchain.renderFinishedSemaphores);
    gfxSwapchain.commandBuffers = GFX_MALLOC(gfxSwapchain.framesInFlight * sizeof *gfxSwapchain.commandBuffers);
    gfxSwapchain.inFlightSemaphores = GFX_MALLOC(gfxSwapchain.framesInFlight * sizeof *gfxSwapchain.inFlightSemaphores);
    gfxSwapchain.inFlightFences = GFX_MALLOC(gfxSwapchain.framesInFlight * sizeof *gfxSwapchain.inFlightFences);

    VK_CHECK(vkAllocateCommandBuffers(gfxDevice.device, &ai, gfxSwapchain.commandBuffers));

    VkSemaphoreCreateInfo sci = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fci = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (uint32_t i = 0; i < gfxSwapchain.imageCount; i++) {
        VK_CHECK(vkCreateSemaphore(gfxDevice.device, &sci, NULL, &gfxSwapchain.renderFinishedSemaphores[i]));
    }

    for (uint32_t i = 0; i < gfxSwapchain.framesInFlight; i++) {
        VK_CHECK(vkCreateSemaphore(gfxDevice.device, &sci, NULL, &gfxSwapchain.inFlightSemaphores[i]));
        VK_CHECK(vkCreateFence(gfxDevice.device, &fci, NULL, &gfxSwapchain.inFlightFences[i]));
    }
}

static void destroySyncObjects()
{
    vkDeviceWaitIdle(gfxDevice.device);

    vkFreeCommandBuffers(gfxDevice.device, gfxDevice.commandPool, gfxSwapchain.framesInFlight,
                         gfxSwapchain.commandBuffers);

    for (uint32_t i = 0; i < gfxSwapchain.imageCount; i++) {
        vkDestroySemaphore(gfxDevice.device, gfxSwapchain.renderFinishedSemaphores[i], NULL);
    }

    for (uint32_t i = 0; i < gfxSwapchain.framesInFlight; i++) {
        vkDestroySemaphore(gfxDevice.device, gfxSwapchain.inFlightSemaphores[i], NULL);
        vkDestroyFence(gfxDevice.device, gfxSwapchain.inFlightFences[i], NULL);
    }

    GFX_FREE(gfxSwapchain.renderFinishedSemaphores);
    GFX_FREE(gfxSwapchain.commandBuffers);
    GFX_FREE(gfxSwapchain.inFlightSemaphores);
    GFX_FREE(gfxSwapchain.inFlightFences);
}

void gfxCreateSwapchain(uint32_t framesInFlight, void (*framebufferSizeCallback)(uint32_t*, uint32_t*))
{
    GFX_RESET(&gfxSwapchain);

    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    if (!framebufferSizeCallback) {
        GFX_ERROR("Framebuffer size callback function must be specified");
    }

    gfxSwapchain.framesInFlight = framesInFlight;
    gfxSwapchain.framebufferSizeCallback = framebufferSizeCallback;

    querySupport();
    createSwapchain();
    createSyncObjects();
}

void gfxDestroySwapchain()
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    vkDeviceWaitIdle(gfxDevice.device);

    destroySyncObjects();
    destroySwapchain();

    GFX_FREE(gfxSwapchain.supportDetails.formats);
    GFX_FREE(gfxSwapchain.supportDetails.presentModes);

    GFX_RESET(&gfxSwapchain);
}

void gfxRecreateSwapchain()
{
    uint32_t width, height;
    gfxSwapchain.framebufferSizeCallback(&width, &height);

    GFX_DEBUG("Recreating swapchain %" PRIu32 "x%" PRIu32, width, height);

    destroySyncObjects();
    destroySwapchain();

    GFX_FREE(gfxSwapchain.supportDetails.formats);
    GFX_FREE(gfxSwapchain.supportDetails.presentModes);

    querySupport();
    createSwapchain();
    createSyncObjects();

    gfxSwapchain.recreated = true;
}

VkCommandBuffer gfxAcquireNextImage()
{
    // Wait for the current frame to not be in flight
    VK_CHECK(vkWaitForFences(gfxDevice.device, 1, &gfxSwapchain.inFlightFences[gfxSwapchain.inFlightIndex], VK_TRUE,
                             UINT64_MAX));
    VK_CHECK(vkResetFences(gfxDevice.device, 1, &gfxSwapchain.inFlightFences[gfxSwapchain.inFlightIndex]));

    // Acquire index of next image in the swapchain
    VkResult result = vkAcquireNextImageKHR(gfxDevice.device, gfxSwapchain.swapchain, UINT64_MAX,
                                            gfxSwapchain.inFlightSemaphores[gfxSwapchain.inFlightIndex], NULL,
                                            &gfxSwapchain.imageIndex);

    // Check if swapchain needs to be reconstructed
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        gfxRecreateSwapchain();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        GFX_ERROR("Failed to acquire next swapchain image");
    }

    VkCommandBufferBeginInfo bi = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VK_CHECK(vkBeginCommandBuffer(gfxSwapchain.commandBuffers[gfxSwapchain.inFlightIndex], &bi));

    return gfxSwapchain.commandBuffers[gfxSwapchain.inFlightIndex];
}

void gfxPresent(VkCommandBuffer cmd, GfxImage* pImage)
{
    if (gfxSwapchain.recreated) {
        gfxSwapchain.recreated = false;
    }

    // Transition swapchain image for blitting
    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, gfxSwapchain.images[gfxSwapchain.imageIndex], NULL);

    // Blit image to current swapchain image
    VkImageSubresourceLayers subresourceLayers = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    int32_t srcWidth = pImage->width;
    int32_t srcHeight = pImage->height;
    int32_t dstWidth = (int32_t)(gfxSwapchain.extent.width);
    int32_t dstHeight = (int32_t)(gfxSwapchain.extent.height);
    VkImageBlit2 region = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
        .srcSubresource = subresourceLayers,
        .srcOffsets = {{0, 0, 0}, {srcWidth, srcHeight, 1}},
        .dstSubresource = subresourceLayers,
        .dstOffsets = {{0, 0, 0}, {dstWidth, dstHeight, 1}},
    };

    VkBlitImageInfo2 blitImageInfo = {
        .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
        .srcImage = pImage->image,
        .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .dstImage = gfxSwapchain.images[gfxSwapchain.imageIndex],
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regionCount = 1,
        .pRegions = &region,
        .filter = VK_FILTER_NEAREST,
    };

    vkCmdBlitImage2(cmd, &blitImageInfo);

    // Transition swapchain image for presenting
    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, gfxSwapchain.images[gfxSwapchain.imageIndex], NULL);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo si = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &gfxSwapchain.inFlightSemaphores[gfxSwapchain.inFlightIndex],
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &gfxSwapchain.renderFinishedSemaphores[gfxSwapchain.imageIndex],
    };

    VK_CHECK(vkQueueSubmit(gfxDevice.queue, 1, &si, gfxSwapchain.inFlightFences[gfxSwapchain.inFlightIndex]));

    VkPresentInfoKHR pi = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &gfxSwapchain.renderFinishedSemaphores[gfxSwapchain.imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &gfxSwapchain.swapchain,
        .pImageIndices = &gfxSwapchain.imageIndex,
    };

    VkResult result = vkQueuePresentKHR(gfxDevice.queue, &pi);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        gfxRecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        GFX_ERROR("Failed to present swapchain image");
    }

    gfxSwapchain.inFlightIndex = (gfxSwapchain.inFlightIndex + 1) % gfxSwapchain.framesInFlight;
}

void gfxCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, GfxBuffer* pBuffer)
{
    GFX_RESET(pBuffer);

    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    *pBuffer = (GfxBuffer){
        .usage = usage,
        .properties = properties,
        .pHostMap = NULL,
    };

    VkBufferCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VK_CHECK(vkCreateBuffer(gfxDevice.device, &ci, NULL, &pBuffer->buffer));

    pBuffer->bufferInfo.buffer = pBuffer->buffer;

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(gfxDevice.device, pBuffer->buffer, &memReqs);

    pBuffer->size = memReqs.size;

    VkMemoryAllocateFlagsInfo allocFlagInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
    };

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = gfxFindMemoryType(memReqs.memoryTypeBits, properties),
    };

    if (pBuffer->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocInfo.pNext = &allocFlagInfo;
    }

    VK_CHECK(vkAllocateMemory(gfxDevice.device, &allocInfo, NULL, &pBuffer->memory));

    VK_CHECK(vkBindBufferMemory(gfxDevice.device, pBuffer->buffer, pBuffer->memory, 0));

    // Map memory if memory is host coherent
    if (pBuffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        VK_CHECK(vkMapMemory(gfxDevice.device, pBuffer->memory, 0, size, 0, &pBuffer->pHostMap));
    }
}

void gfxDestroyBuffer(GfxBuffer* pBuffer)
{
    vkDeviceWaitIdle(gfxDevice.device);

    if (pBuffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        vkUnmapMemory(gfxDevice.device, pBuffer->memory);
    }

    vkDestroyBuffer(gfxDevice.device, pBuffer->buffer, NULL);
    vkFreeMemory(gfxDevice.device, pBuffer->memory, NULL);

    GFX_RESET(pBuffer);
}

void gfxCopyBufferFromHost(const GfxBuffer* pBuffer, const void* pData, VkDeviceSize size, VkDeviceSize offset)
{
    // Check if we need a staging buffer or not
    if (!(pBuffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        // Set up staging buffer
        GfxBuffer staging;
        gfxCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging);

        // Copy to staging buffer
        gfxCopyBufferFromHost(&staging, pData, size, 0);

        // Transfer from staging buffer to this buffer
        VkCommandBuffer cmd = gfxCmdBegin();

        VkBufferCopy region = {
            .dstOffset = offset,
            .size = size,
        };
        vkCmdCopyBuffer(cmd, staging.buffer, pBuffer->buffer, 1, &region);

        gfxCmdEnd(cmd);

        gfxDestroyBuffer(&staging);
    } else {
        // Transfer directly without staging buffer
        char* pOffsettedHostMap = (char*)pBuffer->pHostMap + offset;
        memcpy(pOffsettedHostMap, pData, size);
    }
}

VkWriteDescriptorSet gfxGetBufferDescriptor(GfxBuffer* pBuffer, uint32_t binding, VkDescriptorType type,
                                            VkDeviceSize offset, VkDeviceSize range)
{
    pBuffer->bufferInfo = (VkDescriptorBufferInfo){
        .buffer = pBuffer->buffer,
        .offset = offset,
        .range = range,
    };

    return (VkWriteDescriptorSet){
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &pBuffer->bufferInfo,
    };
}

void gfxCreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format,
                    VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, GfxImage* pImage)
{
    GFX_RESET(pImage);

    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    *pImage = (GfxImage){
        .width = width,
        .height = height,
        .mipLevels = mipLevels,
        .format = format,
        .tiling = tiling,
    };

    VkImageCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {.width = width, .height = height, .depth = 1},
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = samples,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VK_CHECK(vkCreateImage(gfxDevice.device, &ci, NULL, &pImage->image));

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(gfxDevice.device, pImage->image, &memReqs);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = gfxFindMemoryType(memReqs.memoryTypeBits, properties),
    };

    VK_CHECK(vkAllocateMemory(gfxDevice.device, &allocInfo, NULL, &pImage->memory));

    VK_CHECK(vkBindImageMemory(gfxDevice.device, pImage->image, pImage->memory, 0));
}

void gfxDestroyImage(GfxImage* pImage)
{
    vkDeviceWaitIdle(gfxDevice.device);

    vkFreeMemory(gfxDevice.device, pImage->memory, NULL);
    vkDestroyImageView(gfxDevice.device, pImage->imageView, NULL);
    vkDestroyImage(gfxDevice.device, pImage->image, NULL);

    GFX_RESET(pImage);
}

void gfxCreateImageView(GfxImage* pImage, VkImageAspectFlags aspectFlags)
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    VkImageViewCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = pImage->image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = pImage->format,
        .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {.aspectMask = aspectFlags,
                             .baseMipLevel = 0,
                             .levelCount = pImage->mipLevels,
                             .baseArrayLayer = 0,
                             .layerCount = 1},
    };

    VK_CHECK(vkCreateImageView(gfxDevice.device, &ci, NULL, &pImage->imageView));
}

static void generateMipmaps(GfxTexture* pTexture)
{
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(gfxDevice.physicalDevice, pTexture->image.format, &props);

    if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        GFX_ERROR("Texture image format does not support linear blitting");
    }

    VkCommandBuffer cmd = gfxCmdBegin();

    VkImageSubresourceRange subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    int32_t mipWidth = pTexture->image.width;
    int32_t mipHeight = pTexture->image.height;

    for (uint32_t i = 1; i < pTexture->image.mipLevels; i++) {
        subresourceRange.baseMipLevel = i - 1;

        gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR, VK_ACCESS_TRANSFER_WRITE_BIT,
                        VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        pTexture->image.image, &subresourceRange);

        VkImageBlit2 region = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                               .mipLevel = i - 1,
                               .baseArrayLayer = 0,
                               .layerCount = 1},
            .srcOffsets = {{0, 0, 0}, {mipWidth, mipHeight, 1}},
            .dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                               .mipLevel = i,
                               .baseArrayLayer = 0,
                               .layerCount = 1},
            .dstOffsets = {{0, 0, 0}, {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}},
        };

        VkBlitImageInfo2 blitImageInfo = {
            .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            .srcImage = pTexture->image.image,
            .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstImage = pTexture->image.image,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &region,
            .filter = VK_FILTER_LINEAR,
        };

        vkCmdBlitImage2(cmd, &blitImageInfo);

        gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR, VK_ACCESS_2_TRANSFER_READ_BIT,
                        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        pTexture->image.image, &subresourceRange);

        if (mipWidth > 1) {
            mipWidth /= 2;
        }
        if (mipHeight > 1) {
            mipHeight /= 2;
        }
    }

    subresourceRange.baseMipLevel = pTexture->image.mipLevels - 1;

    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    pTexture->image.image, &subresourceRange);

    gfxCmdEnd(cmd);
}

static void createTexture(GfxTexture* pTexture, enum GfxTextureType type, VkFormat format, const void* pData,
                          uint32_t width, uint32_t height, uint32_t channels, bool mipmaps)
{
    uint32_t mipLevels = 1;
    if (mipmaps) {
        mipLevels = (uint32_t)floor(log2(GFX_MAX(width, height))) + 1;
    }

    gfxCreateImage(width, height, mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pTexture->image);

    if (pData) {
        VkDeviceSize size = width * height * channels;

        GfxBuffer staging;
        gfxCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging);

        gfxCopyBufferFromHost(&staging, pData, size, 0);

        VkCommandBuffer cmd = gfxCmdBegin();

        gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                        VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        pTexture->image.image, NULL);

        VkBufferImageCopy region = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .mipLevel = 0,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1},
            .imageOffset = {0, 0},
            .imageExtent = {width, height, 1},
        };

        vkCmdCopyBufferToImage(cmd, staging.buffer, pTexture->image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &region);

        gfxCmdEnd(cmd);

        if (mipmaps) {
            generateMipmaps(pTexture);
        } else {
            cmd = gfxCmdBegin();
            gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            pTexture->image.image, NULL);
            gfxCmdEnd(cmd);
        }

        gfxDestroyBuffer(&staging);
    }

    gfxCreateImageView(&pTexture->image, VK_IMAGE_ASPECT_COLOR_BIT);

    pTexture->imageInfo = (VkDescriptorImageInfo){
        .sampler = NULL,
        .imageView = pTexture->image.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}

static void setDefaultSampler(GfxTexture* pTexture)
{
    pTexture->sampler = VK_NULL_HANDLE;
}

static void createSampler(GfxTexture* pTexture)
{
    if (pTexture->sampler) {
        /* Delete old sampler first */
        vkDeviceWaitIdle(gfxDevice.device);
        vkDestroySampler(gfxDevice.device, pTexture->sampler, NULL);
    } else {
        /* First time, use defaults */
        pTexture->magFilter = VK_FILTER_LINEAR;
        pTexture->minFilter = VK_FILTER_LINEAR;
        pTexture->addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        pTexture->addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        pTexture->addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        pTexture->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
    VkSamplerCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = pTexture->magFilter,
        .minFilter = pTexture->minFilter,
        .addressModeU = pTexture->addressModeU,
        .addressModeV = pTexture->addressModeV,
        .addressModeW = pTexture->addressModeW,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = gfxDevice.properties.physicalDevice.limits.maxSamplerAnisotropy,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = pTexture->mipmapMode,
        .minLod = 0.0f,
        .maxLod = (float)pTexture->image.mipLevels,
        .mipLodBias = 0.0f,
    };
    VK_CHECK(vkCreateSampler(gfxDevice.device, &ci, NULL, &pTexture->sampler));
    pTexture->imageInfo.sampler = pTexture->sampler;
}

void gfxCreateTexture(enum GfxTextureType type, VkFormat format, const void* pData, uint32_t width, uint32_t height,
                      uint32_t channels, bool generateMipmaps, GfxTexture* pTexture)
{
    GFX_RESET(pTexture);

    createTexture(pTexture, type, format, pData, width, height, channels, generateMipmaps);
    createSampler(pTexture);
}

#ifdef GFX_USE_STB_IMAGE
void gfxCreateTextureFromFile(enum GfxTextureType type, VkFormat format, const char* pPath, bool generateMipmaps,
                              GfxTexture* pTexture)
{
    GFX_RESET(pTexture);

    GFX_INFO("Loading texture from %s", pPath);

    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;

    stbi_uc* pData = stbi_load(pPath, &width, &height, &channels, STBI_rgb_alpha);
    channels = STBI_rgb_alpha;

    if (!pData) {
        GFX_ERROR("Failed to load texture.");
        return;
    }

    createTexture(pTexture, type, format, pData, width, height, channels, generateMipmaps);

    stbi_image_free(pData);

    createSampler(pTexture);
}
#endif

void gfxDestroyTexture(GfxTexture* pTexture)
{
    vkDeviceWaitIdle(gfxDevice.device);
    vkDestroySampler(gfxDevice.device, pTexture->sampler, NULL);
    gfxDestroyImage(&pTexture->image);

    GFX_RESET(pTexture);
}

VkWriteDescriptorSet gfxGetTextureDescriptor(const GfxTexture* pTexture, uint32_t binding, VkDescriptorType type)
{
    return (VkWriteDescriptorSet){
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = type,
        .pImageInfo = &pTexture->imageInfo,
    };
}

void gfxSetTextureMagFilter(GfxTexture* pTexture, VkFilter magFilter)
{
    pTexture->magFilter = magFilter;
    createSampler(pTexture);
}

void gfxSetTextureMinFilter(GfxTexture* pTexture, VkFilter minFilter)
{
    pTexture->minFilter = minFilter;
    createSampler(pTexture);
}

void gfxSetTextureMipmapMode(GfxTexture* pTexture, VkSamplerMipmapMode mipmapMode)
{
    pTexture->mipmapMode = mipmapMode;
    createSampler(pTexture);
}

void gfxSetTextureAddressMode(GfxTexture* pTexture, VkSamplerAddressMode modeU, VkSamplerAddressMode modeV,
                              VkSamplerAddressMode modeW)
{
    pTexture->addressModeU = modeU;
    pTexture->addressModeV = modeV;
    pTexture->addressModeW = modeW;
    createSampler(pTexture);
}

static createAttachment(GfxAttachment* pAttachment, uint32_t colorAttachmentCount, GfxImage* pColorAttachments,
                        GfxImage* pDepthAttachment, GfxImage* pResolveAttachment)
{
    *pAttachment = (GfxAttachment){
        .pRenderingAttachmentInfos = GFX_MALLOC(colorAttachmentCount * sizeof(VkRenderingAttachmentInfo)),
        .pFormats = GFX_MALLOC(colorAttachmentCount * sizeof(VkFormat)),
        .pColorAttachments = pColorAttachments,
        .colorAttachmentCount = colorAttachmentCount,
        .pDepthAttachment = pDepthAttachment,
        .pResolveAttachment = pResolveAttachment,
    };

    for (uint32_t i = 0; i < colorAttachmentCount; i++) {
        pAttachment->pRenderingAttachmentInfos[i] = (VkRenderingAttachmentInfo){
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = pAttachment->pColorAttachments[i].imageView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = pAttachment->pResolveAttachment ? VK_RESOLVE_MODE_AVERAGE_BIT : VK_RESOLVE_MODE_NONE,
            .resolveImageView = pAttachment->pResolveAttachment ? pAttachment->pResolveAttachment->imageView : NULL,
            .resolveImageLayout =
                pAttachment->pResolveAttachment ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };
        pAttachment->pFormats[i] = pAttachment->pColorAttachments[i].format;
    }

    pAttachment->pipelineRenderingCreateInfo = (VkPipelineRenderingCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = colorAttachmentCount,
        .pColorAttachmentFormats = pAttachment->pFormats,
        .depthAttachmentFormat = pDepthAttachment->format,
    };
}

void gfxCreateAttachment(uint32_t colorAttachmentCount, GfxImage* pColorAttachments, GfxImage* pDepthAttachment,
                         GfxImage* pResolveAttachment, GfxAttachment* pAttachment)
{
    GFX_RESET(pAttachment);

    createAttachment(pAttachment, colorAttachmentCount, pColorAttachments, pDepthAttachment, pResolveAttachment);
}

void gfxDestroyAttachment(GfxAttachment* pAttachment)
{
    GFX_FREE(pAttachment->pRenderingAttachmentInfos);
    GFX_FREE(pAttachment->pFormats);

    GFX_RESET(pAttachment);
}

void gfxRecreateAttachment(GfxAttachment* pAttachment, uint32_t colorAttachmentCount, GfxImage* pColorAttachments,
                           GfxImage* pDepthAttachment, GfxImage* pResolveAttachment)
{
    GFX_FREE(pAttachment->pRenderingAttachmentInfos);
    GFX_FREE(pAttachment->pFormats);
    createAttachment(pAttachment, colorAttachmentCount, pColorAttachments, pDepthAttachment, pResolveAttachment);
}

void gfxCmdBeginRendering(VkCommandBuffer cmd, const GfxAttachment* pAttachment, VkClearValue clearValue,
                          VkAttachmentLoadOp loadOp)
{
    for (uint32_t i = 0; i < pAttachment->colorAttachmentCount; i++) {
        pAttachment->pRenderingAttachmentInfos[i].clearValue = clearValue;
        pAttachment->pRenderingAttachmentInfos[i].loadOp = loadOp;
    }

    VkRenderingAttachmentInfo depthAttachmentInfo;
    if (pAttachment->pDepthAttachment) {
        depthAttachmentInfo = (VkRenderingAttachmentInfo){
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = pAttachment->pDepthAttachment->imageView,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue.depthStencil = {.depth = 0.0f, .stencil = 0},
        };
    }

    VkExtent2D extent = {
        .width = pAttachment->pColorAttachments[0].width,
        .height = pAttachment->pColorAttachments[0].height,
    };

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {.offset = {0, 0}, .extent = extent},
        .layerCount = 1,
        .colorAttachmentCount = pAttachment->colorAttachmentCount,
        .pColorAttachments = pAttachment->pRenderingAttachmentInfos,
        .pDepthAttachment = pAttachment->pDepthAttachment ? &depthAttachmentInfo : NULL,
        .pStencilAttachment = NULL,
    };

    vkCmdBeginRendering(cmd, &renderingInfo);
}

void gfxCmdEndRendering(VkCommandBuffer cmd, const GfxAttachment* pAttachment)
{
    GFX_UNUSED(pAttachment);
    vkCmdEndRendering(cmd);
}

void gfxCreateLayout(uint32_t bindingCount, VkDescriptorType* pTypes, VkShaderStageFlags* pStages, uint32_t* pCounts,
                     uint32_t pushConstantRangeCount, VkPushConstantRange* pPushConstantRanges, GfxLayout* pLayout)
{
    GFX_RESET(pLayout);

    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    size_t pushConstantRangeSize = pushConstantRangeCount * sizeof(VkPushConstantRange);

    *pLayout = (GfxLayout){
        .pushConstantRangeCount = pushConstantRangeCount,
    };

    if (pLayout->pushConstantRangeCount) {
        pLayout->pPushConstantRanges = GFX_MALLOC(pushConstantRangeSize);
        memcpy(pLayout->pPushConstantRanges, pPushConstantRanges, pushConstantRangeSize);
    }

    VkDescriptorSetLayoutBinding* pBindings = GFX_MALLOC(bindingCount * sizeof *pBindings);

    for (uint32_t i = 0; i < bindingCount; i++) {
        pBindings[i] = (VkDescriptorSetLayoutBinding){
            .binding = i,
            .descriptorType = pTypes[i],
            .descriptorCount = pCounts[i],
            .stageFlags = pStages[i],
        };
    }

    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
        .bindingCount = bindingCount,
        .pBindings = pBindings,
    };

    VK_CHECK(vkCreateDescriptorSetLayout(gfxDevice.device, &setLayoutCreateInfo, NULL, &pLayout->setLayout));

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &pLayout->setLayout,
        .pushConstantRangeCount = pLayout->pushConstantRangeCount,
        .pPushConstantRanges = pLayout->pPushConstantRanges,
    };
    VK_CHECK(vkCreatePipelineLayout(gfxDevice.device, &pipelineLayoutCreateInfo, NULL, &pLayout->pipelineLayout));

    GFX_FREE(pBindings);
}

void gfxDestroyLayout(GfxLayout* pLayout)
{
    vkDeviceWaitIdle(gfxDevice.device);

    vkDestroyPipelineLayout(gfxDevice.device, pLayout->pipelineLayout, NULL);
    vkDestroyDescriptorSetLayout(gfxDevice.device, pLayout->setLayout, NULL);

    if (pLayout->pPushConstantRanges) {
        GFX_FREE(pLayout->pPushConstantRanges);
    }

    GFX_RESET(pLayout);
}

void createShader(GfxShader* pShader, const void* pCode, size_t codeSize, VkShaderStageFlagBits stage,
                  VkShaderStageFlags nextStage, const GfxLayout* pLayout)
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    pShader->pCode = GFX_MALLOC(codeSize);
    memcpy(pShader->pCode, pCode, codeSize);

    pShader->createInfo = (VkShaderCreateInfoEXT){
        .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
        .flags = 0,
        .stage = stage,
        .nextStage = nextStage,
        .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
        .codeSize = codeSize,
        .pCode = pShader->pCode,
        .pName = "main",
        .setLayoutCount = 1,
        .pSetLayouts = &pLayout->setLayout,
        .pushConstantRangeCount = pLayout->pushConstantRangeCount,
        .pPushConstantRanges = pLayout->pPushConstantRanges,
        .pSpecializationInfo = NULL,
    };
}

void gfxCreateShader(const void* pCode, size_t codeSize, VkShaderStageFlagBits stage, VkShaderStageFlags nextStage,
                     const GfxLayout* pLayout, GfxShader* pShader)
{
    GFX_RESET(pShader);

    createShader(pShader, pCode, codeSize, stage, nextStage, pLayout);
}

void gfxCreateShaderFromFileGLSL(const char* pPath, VkShaderStageFlagBits stage, VkShaderStageFlags nextStage,
                                 const GfxLayout* pLayout, GfxShader* pShader)
{
    GFX_RESET(pShader);

    size_t sz = strlen(pPath) + 1;

    *pShader = (GfxShader){
        .pPath = GFX_MALLOC(sz),
    };

    memset(pShader->pPath, 0, sz);
    strcpy_s(pShader->pPath, sz, pPath);

    // Read file
    FILE* file = fopen(pShader->pPath, "rb");
    if (!file) {
        GFX_ERROR("Unable to open %s\n", pShader->pPath);
    }

    long file_size;
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    char* pShaderSource = GFX_MALLOC(file_size + 1);
    fread(pShaderSource, 1, file_size, file);
    pShaderSource[file_size] = 0;

    fclose(file);

    // Compile GLSL to SPIR-V
    glslang_target_client_version_t glslangVersion = GLSLANG_TARGET_VULKAN_1_0;
    switch (VK_VERSION_MAJOR(gfxDevice.apiVersion)) {
    case 1:
        switch (VK_VERSION_MINOR(gfxDevice.apiVersion)) {
        case 0:
            glslangVersion = GLSLANG_TARGET_VULKAN_1_0;
            break;
        case 1:
            glslangVersion = GLSLANG_TARGET_VULKAN_1_1;
            break;
        case 2:
            glslangVersion = GLSLANG_TARGET_VULKAN_1_2;
            break;
        case 3:
            glslangVersion = GLSLANG_TARGET_VULKAN_1_3;
            break;
        case 4:
            glslangVersion = GLSLANG_TARGET_VULKAN_1_4;
            break;
        }
        break;
    }

    glslang_stage_t glslangStage = GLSLANG_STAGE_VERTEX;
    switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
        glslangStage = GLSLANG_STAGE_VERTEX;
        break;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        glslangStage = GLSLANG_STAGE_TESSCONTROL;
        break;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        glslangStage = GLSLANG_STAGE_TESSEVALUATION;
        break;
    case VK_SHADER_STAGE_GEOMETRY_BIT:
        glslangStage = GLSLANG_STAGE_GEOMETRY;
        break;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        glslangStage = GLSLANG_STAGE_FRAGMENT;
        break;
    case VK_SHADER_STAGE_COMPUTE_BIT:
        glslangStage = GLSLANG_STAGE_COMPUTE;
        break;
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        glslangStage = GLSLANG_STAGE_RAYGEN;
        break;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        glslangStage = GLSLANG_STAGE_INTERSECT;
        break;
    case VK_SHADER_STAGE_ANY_HIT_BIT_NV:
        glslangStage = GLSLANG_STAGE_ANYHIT;
        break;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        glslangStage = GLSLANG_STAGE_CLOSESTHIT;
        break;
    case VK_SHADER_STAGE_MISS_BIT_KHR:
        glslangStage = GLSLANG_STAGE_MISS;
        break;
    case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
        glslangStage = GLSLANG_STAGE_CALLABLE;
        break;
    case VK_SHADER_STAGE_TASK_BIT_EXT:
        glslangStage = GLSLANG_STAGE_TASK;
        break;
    case VK_SHADER_STAGE_MESH_BIT_EXT:
        glslangStage = GLSLANG_STAGE_MESH;
        break;
    }

    const glslang_input_t input = {
        .language = GLSLANG_SOURCE_GLSL,
        .stage = glslangStage,
        .client = GLSLANG_CLIENT_VULKAN,
        .client_version = glslangVersion,
        .target_language = GLSLANG_TARGET_SPV,
        .target_language_version = GLSLANG_TARGET_SPV_1_6,
        .code = pShaderSource,
        .default_version = 100,
        .default_profile = GLSLANG_NO_PROFILE,
        .force_default_version_and_profile = false,
        .forward_compatible = false,
        .messages = GLSLANG_MSG_DEFAULT_BIT,
        .resource = glslang_default_resource(),
    };

    glslang_shader_t* shader = glslang_shader_create(&input);

    if (!glslang_shader_preprocess(shader, &input)) {
        GFX_ERROR("GLSL preprocessing failed %s\n%s\n%s", pPath, glslang_shader_get_info_log(shader),
                  glslang_shader_get_info_debug_log(shader));
    }

    if (!glslang_shader_parse(shader, &input)) {
        GFX_ERROR("GLSL parsing failed %s\n%s\n%s\n%s", pPath, glslang_shader_get_info_log(shader),
                  glslang_shader_get_info_debug_log(shader), glslang_shader_get_preprocessed_code(shader));
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
        GFX_ERROR("GLSL linking failed %s\n%s\n%s", pPath, glslang_program_get_info_log(program),
                  glslang_program_get_info_debug_log(program));
    }

    glslang_program_SPIRV_generate(program, glslangStage);

    size_t codeSize = glslang_program_SPIRV_get_size(program) * sizeof(uint32_t);
    uint32_t* pCode = GFX_MALLOC(codeSize);
    memset(pCode, 0, codeSize);
    glslang_program_SPIRV_get(program, pCode);

    const char* spirvMessages = glslang_program_SPIRV_get_messages(program);
    if (spirvMessages) {
        GFX_ERROR("(%s) %s", pPath, spirvMessages);
    }

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    createShader(pShader, pCode, codeSize, stage, nextStage, pLayout);

    GFX_FREE(pCode);
    GFX_FREE(pShaderSource);
}

void gfxDestroyShader(GfxShader* pShader)
{
    vkDeviceWaitIdle(gfxDevice.device);

    VK_LOAD(vkDestroyShaderEXT);
    XvkDestroyShaderEXT(gfxDevice.device, pShader->shader, NULL);

    GFX_FREE(pShader->pCode);
    GFX_FREE(pShader->pPath);

    GFX_RESET(pShader);
}

void gfxBuildShader(GfxShader* pShader)
{
    GFX_INFO("Building shader: %s", pShader->pPath);
    VK_LOAD(vkCreateShadersEXT);
    VK_CHECK(XvkCreateShadersEXT(gfxDevice.device, 1, &pShader->createInfo, NULL, &pShader->shader));
}

void gfxBuildLinkedShaders(GfxShader* pVertexShader, GfxShader* pFragmentShader)
{
    if (!pVertexShader || !pFragmentShader) {
        GFX_ERROR("Both pVertexShader and pFragmentShader need to be specified");
    }

    GFX_INFO("Building shaders: %s", pVertexShader->pPath);
    GFX_INFO("                  %s", pFragmentShader->pPath);

    VkShaderCreateInfoEXT createInfos[] = {
        pVertexShader->createInfo,
        pFragmentShader->createInfo,
    };

    for (uint32_t i = 0; i < GFX_ARRAY_LEN(createInfos); i++) {
        createInfos[i].flags |= VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    }

    VkShaderEXT shaders[GFX_ARRAY_LEN(createInfos)];

    VK_LOAD(vkCreateShadersEXT);
    VK_CHECK(XvkCreateShadersEXT(gfxDevice.device, GFX_ARRAY_LEN(createInfos), createInfos, NULL, shaders));

    pVertexShader->shader = shaders[0];
    pFragmentShader->shader = shaders[1];
}

void gfxCmdBindShader(VkCommandBuffer cmd, const GfxShader* pShader)
{
    VK_LOAD(vkCmdBindShadersEXT);
    XvkCmdBindShadersEXT(cmd, 1, &pShader->createInfo.stage, &pShader->shader);
}

void gfxCmdSetDefaultStates(VkCommandBuffer cmd, uint32_t vertexBindingDescriptionCount,
                            const VkVertexInputBindingDescription2EXT* vertexBindingDescriptions,
                            uint32_t vertexAttributeDescriptionCount,
                            const VkVertexInputAttributeDescription2EXT* vertexAttributeDescriptions)
{
    VK_LOAD(vkCmdSetVertexInputEXT);
    VK_LOAD(vkCmdSetRasterizationSamplesEXT);
    VK_LOAD(vkCmdSetSampleMaskEXT);
    VK_LOAD(vkCmdSetAlphaToCoverageEnableEXT);
    VK_LOAD(vkCmdSetPolygonModeEXT);
    VK_LOAD(vkCmdSetLogicOpEnableEXT);
    VK_LOAD(vkCmdSetColorBlendEnableEXT);
    VK_LOAD(vkCmdSetColorWriteMaskEXT);

    const VkViewport viewport = {
        .width = (float)gfxSwapchain.extent.width,
        .height = (float)gfxSwapchain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    const VkRect2D scissor = {.extent = gfxSwapchain.extent};
    vkCmdSetViewportWithCount(cmd, 1, &viewport);
    vkCmdSetScissorWithCount(cmd, 1, &scissor);
    vkCmdSetRasterizerDiscardEnable(cmd, VK_FALSE);

    XvkCmdSetVertexInputEXT(cmd, vertexBindingDescriptionCount, vertexBindingDescriptions,
                            vertexAttributeDescriptionCount, vertexAttributeDescriptions);
    vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vkCmdSetPrimitiveRestartEnable(cmd, VK_FALSE);

    const VkSampleMask sampleMask = 0x1;
    XvkCmdSetRasterizationSamplesEXT(cmd, VK_SAMPLE_COUNT_1_BIT);
    XvkCmdSetSampleMaskEXT(cmd, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    XvkCmdSetAlphaToCoverageEnableEXT(cmd, VK_FALSE);
    XvkCmdSetPolygonModeEXT(cmd, VK_POLYGON_MODE_FILL);
    vkCmdSetCullMode(cmd, VK_FALSE);
    vkCmdSetFrontFace(cmd, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    vkCmdSetDepthTestEnable(cmd, VK_TRUE);
    vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_GREATER);
    vkCmdSetDepthBoundsTestEnable(cmd, VK_FALSE);
    vkCmdSetDepthBiasEnable(cmd, VK_FALSE);
    vkCmdSetStencilTestEnable(cmd, VK_FALSE);

    const VkBool32 colorBlendEnable = VK_FALSE;
    const VkColorComponentFlags colorComponents =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_A_BIT;
    XvkCmdSetLogicOpEnableEXT(cmd, VK_FALSE);
    XvkCmdSetColorBlendEnableEXT(cmd, 0, 1, &colorBlendEnable);
    XvkCmdSetColorWriteMaskEXT(cmd, 0, 1, &colorComponents);

    vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
}

#endif

#ifdef __cplusplus
}
#endif
