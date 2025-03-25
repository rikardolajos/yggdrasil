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
#define YGGDRASIL_PLATFORM_WINDOWS 1
#define YGGDRASIL_PLATFORM_LINUX 2

#ifndef YGGDRASIL_PLATFORM
#if defined(_WIN64)
#define YGGDRASIL_PLATFORM YGGDRASIL_PLATFORM_WINDOWS
#elif defined(__linux__)
#define YGGDRASIL_PLATFORM YGGDRASIL_PLATFORM_LINUX
#else
#error "Unsupported target platform"
#endif
#endif

#define YGGDRASIL_WINDOWS (YGGDRASIL_PLATFORM == YGGDRASIL_PLATFORM_WINDOWS)
#define YGGDRASIL_LINUX (YGGDRASIL_PLATFORM == YGGDRASIL_PLATFORM_LINUX)

#include <vulkan/vulkan.h>

// Define YGGDRASIL_STBI if stb_image.h is available. This allows for texture
// loading from file.
#ifdef YGGDRASIL_STBI
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

#if YGGDRASIL_LINUX
#include <csignal>
#endif

// Get the length of an array. Don't use for pointers!
#define YG_ARRAY_LEN(x) (uint32_t)(sizeof(x) / sizeof *(x))

// Memory allocation macros. Overload to use custom allocations.
#ifndef YG_MALLOC
#define YG_MALLOC(sz) ygCheckedMalloc(sz)
#endif

#ifndef YG_REALLOC
#define YG_REALLOC(p, sz) realloc(p, sz)
#endif

#ifndef YG_FREE
#define YG_FREE(p) free(p)
#endif


// Error handling and logging //

struct VulkanResult {
    VkResult result;
    const char* string;
} ygVulkanResults[] = {
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
    {VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
     "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS"},
    {VK_ERROR_SURFACE_LOST_KHR, "VK_ERROR_SURFACE_LOST_KHR"},
    {VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"},
    {VK_SUBOPTIMAL_KHR, "VK_SUBOPTIMAL_KHR"},
    {VK_ERROR_OUT_OF_DATE_KHR, "VK_ERROR_OUT_OF_DATE_KHR"},
    {VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"},
    {VK_ERROR_VALIDATION_FAILED_EXT, "VK_ERROR_VALIDATION_FAILED_EXT"},
    {VK_ERROR_INVALID_SHADER_NV, "VK_ERROR_INVALID_SHADER_NV"},
    {VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
     "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT"},
    {VK_ERROR_NOT_PERMITTED_EXT, "VK_ERROR_NOT_PERMITTED_EXT"},
    {VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
     "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT"},
    {VK_THREAD_IDLE_KHR, "VK_THREAD_IDLE_KHR"},
    {VK_THREAD_DONE_KHR, "VK_THREAD_DONE_KHR"},
    {VK_OPERATION_DEFERRED_KHR, "VK_OPERATION_DEFERRED_KHR"},
    {VK_OPERATION_NOT_DEFERRED_KHR, "VK_OPERATION_NOT_DEFERRED_KHR"},
    {VK_PIPELINE_COMPILE_REQUIRED_EXT, "VK_PIPELINE_COMPILE_REQUIRED_EXT"},
    {VK_ERROR_OUT_OF_POOL_MEMORY_KHR, "VK_ERROR_OUT_OF_POOL_MEMORY_KHR"},
    {VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR,
     "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR"},
    {VK_ERROR_FRAGMENTATION_EXT, "VK_ERROR_FRAGMENTATION_EXT"},
    {VK_ERROR_INVALID_DEVICE_ADDRESS_EXT,
     "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT"},
};

// Log an info message
#define YG_INFO(fmt, ...) fprintf(stdout, "INFO: " fmt "\n", ##__VA_ARGS__);

#ifdef NDEBUG
#define YG_DEBUG(fmt, ...)
#else
// Log a debug message. Only visible if NDEBUG is not defined.
#define YG_DEBUG(fmt, ...)                                                     \
    fprintf(stdout, "\x1B[1;92mDEBUG: \x1B[0m" fmt "\n", ##__VA_ARGS__);
#endif

// Log a warning message
#define YG_WARNING(fmt, ...)                                                   \
    fprintf(stderr, "\x1B[1;93mWARNING: \x1B[0m%s:%d: " fmt "\n", __FILE__,    \
            __LINE__, ##__VA_ARGS__);

#ifdef NDEBUG
// Log an error message
#define YG_ERROR(fmt, ...)                                                     \
    fprintf(stderr, "\x1B[1;91mERROR: \x1B[0m%s:%d: " fmt "\n", __FILE__,      \
            __LINE__, ##__VA_ARGS__);                                          \
    abort();
#else
#if YGGDRASIL_WINDOWS
// Log an error message. Will cause a breakpoint if NDEBUG is not defined.
#define YG_ERROR(fmt, ...)                                                     \
    fprintf(stderr, "\x1B[1;91mERROR: \x1B[0m%s:%d: " fmt "\n", __FILE__,      \
            __LINE__, ##__VA_ARGS__);                                          \
    __debugbreak();
#elif YGGDRASIL_LINUX
// Log an error message. Will cause a SIGTRAP if NDEBUG is not defined.
#define YG_ERROR(fmt, ...)                                                     \
    fprintf(stderr, "\x1B[1;91mERROR: \x1B[0m%s:%d: " fmt "\n", __FILE__,      \
            __LINE__, ##__VA_ARGS__);                                          \
    std::raise(SIGTRAP);
#endif
#endif

// Check for Vulkan function call for errors
#define VK_CHECK(x)                                                            \
    do {                                                                       \
        VkResult res = x;                                                      \
        if (res) {                                                             \
            for (size_t i = 0; i < YG_ARRAY_LEN(ygVulkanResults); i++) {       \
                if ((res) == ygVulkanResults[i].result) {                      \
                    YG_ERROR("%s", ygVulkanResults[i].string);                 \
                }                                                              \
            }                                                                  \
        }                                                                      \
    } while (0)


// Yggdrasil type definitions //

// Properties of the chosen physical device
typedef struct YgDeviceProperties {
    VkPhysicalDeviceProperties physicalDevice;
    VkPhysicalDeviceMemoryProperties memory;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipeline;
} YgDeviceProperties;

// Device contains a Vulkan context for rendering. The Yggdrasil device is
// monolithic and is setup through ygCreateInstance() and ygCreateDevice().
// Release resources with ygDestroyDevice() and ygDestroyInstance().
typedef struct YgDevice {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    YgDeviceProperties properties;
    VkCommandPool commandPool;
    VkQueue queue;
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    uint32_t queueFamilyIndex;
    bool vsync;
} YgDevice;

// Swapchain abstracts the handling of swapchain images and frames in flight.
// The Yggdrasil swapchain is monolithic and is setup through
// ygCreateSwapchain(). Use ygAcquiteNextImage() and ygPresent() to acquire and
// present images from the swapchain respectively. The swapchain can be
// recreated with ygRecreateSwapchain() if the framebuffer size changed. The
// framebufferSizeCallback() function pointer will be used to retrieve the new
// framebuffer size. Release resources with ygDestroySwapchain().
typedef struct YgSwapchain {
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;
    bool recreated;

    void (*framebufferSizeCallback)(uint32_t*, uint32_t*);

    struct SupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR* formats;
        VkPresentModeKHR* presentModes;
        uint32_t formatCount;
        uint32_t presentCount;
    } supportDetails;

    VkImage* images;
    VkImageView* imageViews;
    uint32_t imageCount;
    uint32_t imageIndex;

    VkCommandBuffer* commandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
    uint32_t framesInFlight;
    uint32_t inFlightIndex;
} YgSwapchain;

// Buffer abstracts a Vulkan buffer and memory allocation. Prefer to use large
// buffers and offsets than many small buffers, that would cause many small
// allocations. Buffers are created with ygCreateBuffer(). Copy data from host
// to the device allocated buffer with ygCopyBufferFromHost(). A buffer with
// host coherent memory will always be mapped on pHostMap. When copying buffers
// from host to device with a non-host coherent memory, a staging buffer will be
// used. Realse resources with ygDestroyDevice().
typedef struct YgBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
    VkDeviceSize size;
    void* pHostMap;
    VkDescriptorBufferInfo bufferInfo;
} YgBuffer;

// Image abstracts a Vulkan image, image view and memory allocation. Use
// ygCreateImage() to create a new image and ygCreateImageView() to create an
// image view for a created image. Release resources with ygDestroyImage().
typedef struct YgImage {
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels;
    VkFormat format;
    VkImageTiling tiling;
} YgImage;

// Sampler abstracts a Vulkan sampler. Create with ygCreateSampler(). Release
// resources with ygDestroySampler().
typedef struct YgSampler {
    VkSampler sampler;
} YgSampler;

// Different textures types for creating textures.
enum YgTextureType {
    YG_TEXTURE_1D,
    YG_TEXTURE_2D,
    YG_TEXTURE_3D,
    YG_TEXTURE_CUBE_MAP,
    YG_TEXTURE_TYPE_COUNT,
};

// Texture is a combination of an Yggdrasil image and sampler. Create a new
// texture with ygCreateTexture() and passing it a pointer to the texture data.
// If stb_image.h is available and YGGDRASIL_STBI has been defined, textures can
// be created from files using ygCreateTextureFromFile(). Mipmaps are
// automatically generated if specified. Use ygSetTextureSampler() to assign a
// sampler for the texture. The write descriptor for the texture can be
// retrieved with ygGetTextureDescriptor(). Release resources with
// ygDestroyTexture().
typedef struct YgTexture {
    YgImage image;
    VkDescriptorImageInfo imageInfo;
} YgTexture;

// Pass abstracts the use of dynamic rendering in Vulkan. Create a new pass with
// ygCreatePass(). Before rendering with the pass call ygBeginPass() and call
// ygEndPass() when done rendering with the pass. If the attachment changes
// (resolution change for instance) then the pass should be recreated.
// ygRecreatePass() can be used, or alternatively ygDestroyPass() and
// ygCreatePass(). Release resources with ygDestroyPass().
typedef struct YgPass {
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo;
    VkRenderingAttachmentInfo* pRenderingAttachmentInfos;
    VkFormat* pFormats;
    YgImage* pColorAttachments;
    uint32_t colorAttachmentCount;
    YgImage* pDepthAttachment;
    YgImage* pResolveAttachment;
} YgPass;


// Monolithic global variables //

extern YgDevice ygDevice;
extern YgSwapchain ygSwapchain;


// Function declarations //

/// <summary>
/// Create a new instance. Handle is internally managed and accessible through
/// ygDevice.
/// </summary>
/// <param name="apiVersion">A Vulkan API version, VK_API_VERSION_* or
/// VK_MAKE_API_VERSION()</param> <param name="ppInstanceExtensions">List of
/// instance extensions to use</param> <param
/// name="instanceExtensionCount">Number of instance extensions</param>
void ygCreateInstance(uint32_t apiVersion, const char** ppInstanceExtensions,
                      uint32_t instanceExtensionCount);

/// <summary>
/// Release resources for the instance.
/// </summary>
void ygDestroyInstance();

/// <summary>
/// Create a new device. Handle is internally managed and accessible through
/// ygDevice.
/// </summary>
/// <param name="physicalDeviceIndex">Physical device index to use</param>
/// <param name="ppDeviceExtensions">List of device extensions to use</param>
/// <param name="deviceExtensionCount">Number of device extensions</param>
/// <param name="features">Pointer to features that will be put in pNext of
/// VkDeviceCreateInfo, can be NULL</param> <param name="surface">Surface to
/// use</param>
void ygCreateDevice(uint32_t physicalDeviceIndex,
                    const char** ppDeviceExtensions,
                    uint32_t deviceExtensionCount,
                    VkPhysicalDeviceFeatures2* features, VkSurfaceKHR surface);

/// <summary>
/// Release resources for the device.
/// </summary>
void ygDestroyDevice();

/// <summary>
/// Get the sample count of the current device.
/// </summary>
/// <returns>Sample count</returns>
VkSampleCountFlagBits ygGetDeviceSampleCount();

/// <summary>
/// Create a new swapchain. Handle is internally managed and accessible through
/// ygSwapchain.
/// </summary>
/// <param name="framesInFlight">Number of frames in flight to use</param>
/// <param name="framebufferSizeCallback">Callback function where the current
/// framebuffer size can be retrieved</param>
void ygCreateSwapchain(uint32_t framesInFlight,
                       void (*framebufferSizeCallback)(uint32_t*, uint32_t*));

/// <summary>
/// Release the resources for the swapchain.
/// </summary>
void ygDestroySwapchain();

/// <summary>
/// Recreate the swapchain, for instance if the framebuffer size changed. New
/// framebuffer size is retrieved through framebufferSizeCallback().
/// </summary>
void ygRecreateSwapchain();

/// <summary>
/// Acquire a new image from the swapchain. This call will block until an image
/// is available.
/// </summary>
/// <returns>Command buffer to use to render to the new image</returns>
VkCommandBuffer ygAcquireNextImage();

/// <summary>
/// Present an image to the swapchain by blitting it.
/// </summary>
/// <param name="cmd">Command buffer retrieved from a call to
/// ygAcquireNextImage()</param> <param name="pImage"></param>
void ygPresent(VkCommandBuffer cmd, YgImage* pImage);

/// <summary>
/// Create a new buffer.
/// </summary>
/// <param name="size">Size of buffer in bytes</param>
/// <param name="usage">How the buffer will be used</param>
/// <param name="properties">Properties of the memory to allocate</param>
/// <param name="pBuffer">Where the created buffer will be stored</param>
void ygCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, YgBuffer* pBuffer);

/// <summary>
/// Release resources for a buffer.
/// </summary>
/// <param name="pBuffer">Buffer to destroy</param>
void ygDestroyBuffer(YgBuffer* pBuffer);

/// <summary>
/// Copy data from host memory to a buffer's device memory. If buffer's memory
/// is not host coherent, a staging buffer will be used.
/// </summary>
/// <param name="pBuffer">Buffer to use</param>
/// <param name="pData">Pointer to data to copy</param>
/// <param name="size">Size of data to copy in bytes</param>
/// <param name="offset">Offset into buffer's device memory to put the
/// data</param>
void ygCopyBufferFromHost(const YgBuffer* pBuffer, const void* pData,
                          VkDeviceSize size, VkDeviceSize offset);

/// <summary>
/// Get the write descriptor of a buffer.
/// </summary>
/// <param name="pBuffer">Buffer to use</param>
/// <param name="binding">In which binding the buffer descriptor should be
/// placed</param> <param name="offset">Offset into buffer to bind</param>
/// <param name="range">Range in bytes to bind, can be VK_WHOLE_SIZE</param>
/// <returns>A write descriptor set</returns>
VkWriteDescriptorSet ygGetBufferDescriptor(YgBuffer* pBuffer, uint32_t binding,
                                           VkDeviceSize offset,
                                           VkDeviceSize range);

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
void ygCreateImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                   VkSampleCountFlagBits samples, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, YgImage* pImage);

/// <summary>
/// Release resources for an image.
/// </summary>
/// <param name="pImage">Image to destroy</param>
void ygDestroyImage(YgImage* pImage);

/// <summary>
/// Create an image view for an image.
/// </summary>
/// <param name="pImage">Image to use</param>
/// <param name="aspectFlags">Aspect flags to use for image view</param>
void ygCreateImageView(YgImage* pImage, VkImageAspectFlags aspectFlags);

/// <summary>
/// Create a new sampler.
/// </summary>
/// <param name="magFilter">Magnification filter to use</param>
/// <param name="minFilter">Minification filter to use</param>
/// <param name="mipmapMode">Mipmap mode to use</param>
/// <param name="addressModeU">Address mode U (wrapping) to use</param>
/// <param name="addressModeV">Address mode V (wrapping) to use</param>
/// <param name="addressModeW">Address mode W (wrapping) to use</param>
/// <param name="pSampler">Where the created sampler will be stored</param>
void ygCreateSampler(VkFilter magFilter, VkFilter minFilter,
                     VkSamplerMipmapMode mipmapMode,
                     VkSamplerAddressMode addressModeU,
                     VkSamplerAddressMode addressModeV,
                     VkSamplerAddressMode addressModeW, YgSampler* pSampler);

/// <summary>
/// Release resources for a sampler.
/// </summary>
/// <param name="pSampler">Sampler to destroy</param>
void ygDestroySampler(YgSampler* pSampler);

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
void ygCreateTexture(enum YgTextureType type, VkFormat format,
                     const void* pData, uint32_t width, uint32_t height,
                     uint32_t channels, bool generateMipmaps,
                     YgTexture* pTexture);

#ifdef YGGDRASIL_STBI
/// <summary>
/// Create a new texture from file.
/// </summary>
/// <param name="type">Type of texture to create</param>
/// <param name="format">Format to use</param>
/// <param name="pPath">Path to texture file</param>
/// <param name="generateMipmaps">Whether to generate mipmaps or not</param>
/// <param name="pTexture">Where the created texture will be stored</param>
void ygCreateTextureFromFile(enum YgTextureType type, VkFormat format,
                             const char* pPath, bool generateMipmaps,
                             YgTexture* pTexture);
#endif

/// <summary>
/// Release resources for a texture.
/// </summary>
/// <param name="pTexture">Texture to destroy</param>
void ygDestroyTexture(YgTexture* pTexture);

/// <summary>
/// Set which sampler to use for a texture.
/// </summary>
/// <param name="pTexture">Texture to use</param>
/// <param name="pSampler">Sampler to use</param>
void ygSetTextureSampler(YgTexture* pTexture, const YgSampler* pSampler);

/// <summary>
/// Get the write descriptor of a texture.
/// </summary>
/// <param name="pTexture">Texture to use</param>
/// <param name="binding">In which binding the texture descriptor should be
/// placed</param> <returns>A write descriptor set</returns>
VkWriteDescriptorSet ygGetTextureDescriptor(const YgTexture* pTexture,
                                            uint32_t binding);

/// <summary>
/// Create a new pass.
/// </summary>
/// <param name="pColorAttachments">List of images that should be used as color
/// attachments</param> <param name="colorAttachmentCount">Number of color
/// attachments</param> <param name="pDepthAttachment">Depth attachment to use,
/// can be NULL</param> <param name="pResolveAttachment">Resolve attachment to
/// use, can be NULL</param> <param name="pPass">Where the created pass will be
/// stored</param>
void ygCreatePass(YgImage* pColorAttachments, uint32_t colorAttachmentCount,
                  YgImage* pDepthAttachment, YgImage* pResolveAttachment,
                  YgPass* pPass);

/// <summary>
/// Release resource for a pass
/// </summary>
/// <param name="pPass">Pass to destroy</param>
void ygDestroyPass(YgPass* pPass);

/// <summary>
/// Recreate a pass if attachments changed. Same as calling ygDestroyPass()
/// followed by ygCreatePass().
/// </summary>
/// <param name="pPass">Pass to recreate</param>
/// <param name="pColorAttachments">List of images that should be used as color
/// attachments</param> <param name="colorAttachmentCount">Number of color
/// attachments</param> <param name="pDepthAttachment">Depth attachment to use,
/// can be NULL</param> <param name="pResolveAttachment">Resolve attachment to
/// use, can be NULL</param>
void ygRecreatePass(YgPass* pPass, YgImage* pColorAttachments,
                    uint32_t colorAttachmentCount, YgImage* pDepthAttachment,
                    YgImage* pResolveAttachment);

/// <summary>
/// Begin dynamic rendering using a pass.
/// </summary>
/// <param name="pPass">Pass to use</param>
/// <param name="cmd">Command buffer to use</param>
/// <param name="clearValue">Clear value for attachments</param>
/// <param name="loadOp">Load operation for attachments</param>
void ygBeginPass(const YgPass* pPass, VkCommandBuffer cmd,
                 VkClearValue clearValue, VkAttachmentLoadOp loadOp);

/// <summary>
/// End dynamic rendering using a pass.
/// </summary>
/// <param name="pPass">Pass to use</param>
/// <param name="cmd">Command buffer to use</param>
void ygEndPass(const YgPass* pPass, VkCommandBuffer cmd);


// Inlined helper functions //

/// <summary>
/// Call malloc and check for valid pointer.
/// </summary>
/// <param name="sz">Size in bytes to allocate</param>
/// <returns>Pointer to allocated memory</returns>
inline void* ygCheckedMalloc(size_t sz)
{
    void* p = malloc(sz);
    if (!p) {
        YG_ERROR("Unable to allocate memory");
    }
    return p;
}

/// <summary>
/// Align value to alignment.
/// </summary>
/// <param name="value">Value to align</param>
/// <param name="alignment">Required alignment</param>
/// <returns></returns>
inline VkDeviceSize ygAlignTo(VkDeviceSize value, VkDeviceSize alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

/// <summary>
/// Create and begin a new one-time-use command buffer.
/// </summary>
/// <returns>A new command buffer</returns>
inline VkCommandBuffer ygCmdBegin()
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    VkCommandBufferAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = ygDevice.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    VK_CHECK(vkAllocateCommandBuffers(ygDevice.device, &ai, &cmd));

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
inline void ygCmdEnd(VkCommandBuffer cmd)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo si = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    VK_CHECK(vkQueueSubmit(ygDevice.queue, 1, &si, NULL));
    VK_CHECK(vkQueueWaitIdle(ygDevice.queue));

    vkFreeCommandBuffers(ygDevice.device, ygDevice.commandPool, 1, &cmd);
}

/// <summary>
/// Find a memory type given a type filter and required memory properties.
/// </summary>
/// <param name="typeFilter">Type filter to use</param>
/// <param name="properties">Memory property flags</param>
/// <returns>Index to memory type</returns>
inline uint32_t ygFindMemoryType(uint32_t typeFilter,
                                 VkMemoryPropertyFlags properties)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    for (uint32_t i = 0; i < ygDevice.properties.memory.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (ygDevice.properties.memory.memoryTypes[i].propertyFlags &
             properties) == properties) {
            return i;
        }
    }

    YG_ERROR("Failed to find suitable memory type");
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
inline VkFormat ygFindSupportedFormat(VkFormat* pCandidates,
                                      uint32_t candidateCount,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    for (uint32_t i = 0; i < candidateCount; i++) {
        VkFormat c = pCandidates[i];

        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(ygDevice.physicalDevice, c,
                                            &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (properties.linearTilingFeatures & features) == features) {
            return c;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (properties.optimalTilingFeatures & features) == features) {
            return c;
        }
    }
    return VK_FORMAT_UNDEFINED;
}

/// <summary>
/// Find a depth format supported by the current device.
/// </summary>
/// <returns>A supported depth format</returns>
inline VkFormat ygFindDepthFormat()
{
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };

    return ygFindSupportedFormat(
        candidates, YG_ARRAY_LEN(candidates), VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

/// <summary>
/// Create an image barrier.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="srcStage">Stage to wait for</param>
/// <param name="srcAccess">Type of access to wait for</param>
/// <param name="dstStage">Stage before which the barrier has to take
/// place</param> <param name="dstAccess">Type of acces before which the barrer
/// has to take place</param> <param name="oldLayout">Previous image
/// layout</param> <param name="newLayout">New image layout</param> <param
/// name="image">Image to use</param> <param
/// name="pSubresourceRange">Subresource range to use, can be NULL in which case
/// a default subrange is used.</param>
inline void ygImageBarrier(VkCommandBuffer cmd, VkPipelineStageFlags2 srcStage,
                           VkAccessFlags2 srcAccess,
                           VkPipelineStageFlags2 dstStage,
                           VkAccessFlags2 dstAccess, VkImageLayout oldLayout,
                           VkImageLayout newLayout, VkImage image,
                           VkImageSubresourceRange* pSubresourceRange)
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
        .subresourceRange =
            pSubresourceRange ? *pSubresourceRange : defaultSubresourceRange,
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
inline void ygTransitionForColorAttachment(VkCommandBuffer cmd, YgImage* pImage)
{
    ygImageBarrier(
        cmd, VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        pImage->image, NULL);
}

/// <summary>
/// Transition the image layout of a color attachment for blitting. Previous
/// layout is assumed to be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pImage">Image to transition</param>
inline void ygTransitionForBlitting(VkCommandBuffer cmd, YgImage* pImage)
{
    ygImageBarrier(cmd, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                   VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                   VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pImage->image, NULL);
}

// Define YGGDRASIL_IMPLEMENTATION in exactly one compilation unit before
// including yggdrasil.h
#ifdef YGGDRASIL_IMPLEMENTATION


// Monolithic global variables //

static YgDevice ygDevice;
static YgSwapchain ygSwapchain;


// Helper macros //

// Reset the memory of an object after releasing its resources.
#define YG_RESET(x) memset((x), 0, sizeof(*(x)))

// Get the max of two values
#define YG_MAX(x, y) ((x) > (y) ? (x) : (y))
// Get the min of two values
#define YG_MIN(x, y) ((x) < (y) ? (x) : (y))
// Clamp a value to an interval
#define YG_CLAMP(x, low, high) (YG_MIN(YG_MAX((x), (low)), (high)))

// Macro for loading a device function pointers as Xvk...()
#define VK_LOAD(func_name)                                                     \
    PFN_##func_name X##func_name =                                             \
        (PFN_##func_name)vkGetDeviceProcAddr(ygDevice.device, #func_name)

// Macro for calling a function via its vkGetDeviceProcAddr name
#define VK_CALL(func_name, ...)                                                \
    do {                                                                       \
        PFN_##func_name pfn_##func_name =                                      \
            (PFN_##func_name)vkGetDeviceProcAddr(ygDevice.device, #func_name); \
        pfn_##func_name(__VA_ARGS__);                                          \
    } while (0);

#ifndef NDEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        YG_WARNING("%s: %s", pCallbackData->pMessageIdName,
                   pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        YG_ERROR("%s: %s", pCallbackData->pMessageIdName,
                 pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}

static VkResult createDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void
destroyDebugUtilsMessengerEXT(VkInstance instance,
                              VkDebugUtilsMessengerEXT debugMessenger,
                              const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) {
        func(instance, debugMessenger, pAllocator);
    }
}

static void
populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* ci)
{
    *ci = (VkDebugUtilsMessengerCreateInfoEXT){
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };
}

static void createDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT ci;
    populateDebugMessengerCreateInfo(&ci);

    VK_CHECK(createDebugUtilsMessengerEXT(ygDevice.instance, &ci, NULL,
                                          &ygDevice.debugMessenger));
}
#endif

void ygCreateInstance(uint32_t apiVersion, const char** ppInstanceExtensions,
                      uint32_t instanceExtensionCount)
{
    VkApplicationInfo ai = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pEngineName = "Yggdrasil",
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
    ci.enabledLayerCount = YG_ARRAY_LEN(layers);
    ci.ppEnabledLayerNames = layers;

    const char** ppExpandedExtensions =
        YG_MALLOC((instanceExtensionCount + 1) * sizeof *ppExpandedExtensions);
    memcpy(ppExpandedExtensions, ppInstanceExtensions,
           instanceExtensionCount * sizeof *ppExpandedExtensions);
    ppExpandedExtensions[instanceExtensionCount] =
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    ci.enabledExtensionCount = instanceExtensionCount + 1;
    ci.ppEnabledExtensionNames = ppExpandedExtensions;
#endif

    VK_CHECK(vkCreateInstance(&ci, NULL, &ygDevice.instance));

    uint32_t version;
    VK_CHECK(vkEnumerateInstanceVersion(&version));
    YG_INFO("Created Vulkan instance: %d.%d.%d", VK_API_VERSION_MAJOR(version),
            VK_API_VERSION_MINOR(version), VK_API_VERSION_PATCH(version));

#ifndef NDEBUG
    createDebugMessenger();
    YG_FREE(ppExpandedExtensions);
#endif
}

void ygDestroyInstance()
{
    if (ygDevice.instance) {
#if defined(_DEBUG)
        destroyDebugUtilsMessengerEXT(ygDevice.instance,
                                      ygDevice.debugMessenger, NULL);
#endif
        vkDestroyInstance(ygDevice.instance, NULL);
    }
}

static bool checkDeviceExtensionSupport(const char** ppDeviceExtensions,
                                        uint32_t deviceExtensionCount)
{
    uint32_t n;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(ygDevice.physicalDevice, NULL,
                                                  &n, NULL));
    VkExtensionProperties* pAvailable = YG_MALLOC(n * sizeof *pAvailable);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(ygDevice.physicalDevice, NULL,
                                                  &n, pAvailable));

    YG_DEBUG("Requesting device extensions (%d):", deviceExtensionCount);
    for (uint32_t i = 0; i < deviceExtensionCount; i++) {
        YG_DEBUG(" * %s", ppDeviceExtensions[i]);
    }

    YG_DEBUG("Available device extensions (%d):", n);
    for (uint32_t i = 0; i < n; i++) {
        YG_DEBUG(" * %s", pAvailable[i].extensionName);
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
            YG_ERROR("The requested extension %s is not available",
                     ppDeviceExtensions[i]);
            result = false;
        }
    }

    YG_FREE(pAvailable);

    return result;
}

static uint32_t getQueueFamilyIndex(VkSurfaceKHR surface,
                                    uint32_t requiredFamilyFlags)
{
    uint32_t n;
    vkGetPhysicalDeviceQueueFamilyProperties(ygDevice.physicalDevice, &n, NULL);
    VkQueueFamilyProperties* pProps = YG_MALLOC(n * sizeof *pProps);
    vkGetPhysicalDeviceQueueFamilyProperties(ygDevice.physicalDevice, &n,
                                             pProps);

    if (!n) {
        YG_ERROR("No Vulkan queue family available");
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

    YG_DEBUG("Available queue families for selected device (%d):", n);
    for (uint32_t i = 0; i < n; i++) {
        YG_DEBUG(" * [%d]:", i);
        for (uint32_t j = 0; j < YG_ARRAY_LEN(vulkanQueues); j++) {
            if (pProps[i].queueFlags & vulkanQueues[j].flagBit) {
                YG_DEBUG("         %s", vulkanQueues[j].string);
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
        YG_ERROR("No Vulkan queue found for requested families");
    }

    // Check that the selected queue family supports PRESENT
    VkBool32 supported;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(ygDevice.physicalDevice,
                                                  index, surface, &supported));
    if (!supported) {
        YG_ERROR("Selected queue family does not support PRESENT");
    }

    YG_FREE(pProps);

    return index;
}

void ygCreateDevice(uint32_t physicalDeviceIndex,
                    const char** ppDeviceExtensions,
                    uint32_t deviceExtensionCount,
                    VkPhysicalDeviceFeatures2* features, VkSurfaceKHR surface)
{
    if (!ygDevice.instance) {
        YG_ERROR("Instance not initialized");
    }

    if (ygDevice.device) {
        YG_ERROR("Device already created");
    }

    ygDevice.surface = surface;

    // Iterate all physical devices
    uint32_t n;
    VK_CHECK(vkEnumeratePhysicalDevices(ygDevice.instance, &n, NULL));
    VkPhysicalDevice* pPhysicalDevices =
        YG_MALLOC(n * sizeof *pPhysicalDevices);
    VK_CHECK(
        vkEnumeratePhysicalDevices(ygDevice.instance, &n, pPhysicalDevices));

    YG_INFO("Available devices (%d):", n);
    for (uint32_t i = 0; i < n; i++) {
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtp = {
            .sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR,
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
            ygDevice.physicalDevice = pPhysicalDevices[i];
            vkGetPhysicalDeviceProperties(ygDevice.physicalDevice,
                                          &ygDevice.properties.physicalDevice);
            vkGetPhysicalDeviceMemoryProperties(ygDevice.physicalDevice,
                                                &ygDevice.properties.memory);
            ygDevice.properties.rayTracingPipeline = rtp;
        }

        YG_INFO(" * [%d] %s, driver: %s %s, Vulkan %d.%d.%d %s", i,
                prop.properties.deviceName, driver.driverName,
                driver.driverInfo,
                VK_API_VERSION_MAJOR(prop.properties.apiVersion),
                VK_API_VERSION_MINOR(prop.properties.apiVersion),
                VK_API_VERSION_PATCH(prop.properties.apiVersion),
                i == physicalDeviceIndex ? "(chosen)" : "");
    }

    YG_FREE(pPhysicalDevices);

    checkDeviceExtensionSupport(ppDeviceExtensions, deviceExtensionCount);

    uint32_t queueFamilyIndex = getQueueFamilyIndex(
        surface,
        VK_QUEUE_GRAPHICS_BIT && VK_QUEUE_COMPUTE_BIT && VK_QUEUE_TRANSFER_BIT);

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

    VK_CHECK(
        vkCreateDevice(ygDevice.physicalDevice, &ci, NULL, &ygDevice.device));

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndex,
    };

    VK_CHECK(vkCreateCommandPool(ygDevice.device, &commandPoolCreateInfo, NULL,
                                 &ygDevice.commandPool));
    vkGetDeviceQueue(ygDevice.device, queueFamilyIndex, 0, &ygDevice.queue);
}

void ygDestroyDevice()
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    vkDeviceWaitIdle(ygDevice.device);

    if (ygDevice.commandPool) {
        vkDestroyCommandPool(ygDevice.device, ygDevice.commandPool, NULL);
    }
    if (ygDevice.device) {
        vkDestroyDevice(ygDevice.device, NULL);
    }
    if (ygDevice.surface) {
        vkDestroySurfaceKHR(ygDevice.instance, ygDevice.surface, NULL);
    }

    YG_RESET(&ygDevice);
}

VkSampleCountFlagBits ygGetDeviceSampleCount()
{
    VkSampleCountFlags counts =
        ygDevice.properties.physicalDevice.limits.framebufferColorSampleCounts &
        ygDevice.properties.physicalDevice.limits.framebufferDepthSampleCounts;

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
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        ygDevice.physicalDevice, ygDevice.surface,
        &ygSwapchain.supportDetails.capabilities));

    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        ygDevice.physicalDevice, ygDevice.surface,
        &ygSwapchain.supportDetails.formatCount, NULL));
    ygSwapchain.supportDetails.formats =
        YG_MALLOC(ygSwapchain.supportDetails.formatCount *
                  sizeof *ygSwapchain.supportDetails.formats);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        ygDevice.physicalDevice, ygDevice.surface,
        &ygSwapchain.supportDetails.formatCount,
        ygSwapchain.supportDetails.formats));

    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        ygDevice.physicalDevice, ygDevice.surface,
        &ygSwapchain.supportDetails.presentCount, NULL));
    ygSwapchain.supportDetails.presentModes =
        YG_MALLOC(ygSwapchain.supportDetails.presentCount *
                  sizeof *&ygSwapchain.supportDetails.presentModes);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        ygDevice.physicalDevice, ygDevice.surface,
        &ygSwapchain.supportDetails.presentCount,
        ygSwapchain.supportDetails.presentModes));
}

// Iterate through available formats and return the one that matches what we
// want. If what we want is not available, the first format in the array is
// returned.
static VkSurfaceFormatKHR
chooseSurfaceFormat(const VkSurfaceFormatKHR* availableFormats,
                    uint32_t availableFormatsCount)
{
    for (uint32_t i = 0; i < availableFormatsCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormats[i].colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }
    return availableFormats[0];
}

// Iterate through available present modes and see if a no v-sync mode is
// available, otherwise go for VK_PRESENT_MODE_FIFO_KHR. If vsync is requested,
// just go for VK_PRESENT_MODE_FIFO_KHR.
static VkPresentModeKHR
choosePresentMode(const VkPresentModeKHR* availablePresentModes,
                  uint32_t availablePresentModeCount, bool vsync)
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
static VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR* capabilities,
                               uint32_t width, uint32_t height)
{
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        VkExtent2D actualExtent = {
            .width = (uint32_t)width,
            .height = (uint32_t)height,
        };

        actualExtent.width =
            YG_CLAMP(actualExtent.width, capabilities->minImageExtent.width,
                     capabilities->maxImageExtent.width);
        actualExtent.height =
            YG_CLAMP(actualExtent.height, capabilities->minImageExtent.height,
                     capabilities->maxImageExtent.height);

        return actualExtent;
    }
}

static void createSwapchain()
{
    uint32_t width;
    uint32_t height;
    ygSwapchain.framebufferSizeCallback(&width, &height);

    VkSurfaceFormatKHR surfaceFormat =
        chooseSurfaceFormat(ygSwapchain.supportDetails.formats,
                            ygSwapchain.supportDetails.formatCount);
    VkPresentModeKHR presentMode = choosePresentMode(
        ygSwapchain.supportDetails.presentModes,
        ygSwapchain.supportDetails.presentCount, ygDevice.vsync);
    VkExtent2D extent =
        chooseExtent(&ygSwapchain.supportDetails.capabilities, width, height);

    // Using at least minImageCount number of images is required but using one
    // extra can avoid unnecessary waits on the driver
    ygSwapchain.imageCount =
        ygSwapchain.supportDetails.capabilities.minImageCount + 1;

    // Also make sure that we are not exceeding the maximum number of images
    if (ygSwapchain.supportDetails.capabilities.maxImageCount > 0 &&
        ygSwapchain.imageCount >
            ygSwapchain.supportDetails.capabilities.maxImageCount) {
        ygSwapchain.imageCount =
            ygSwapchain.supportDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = ygDevice.surface,
        .minImageCount = ygSwapchain.imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1, // Unless rendering stereoscopically
        .imageUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform =
            ygSwapchain.supportDetails.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    ygSwapchain.format = surfaceFormat.format;
    ygSwapchain.extent = extent;

    VK_CHECK(vkCreateSwapchainKHR(ygDevice.device, &ci, NULL,
                                  &ygSwapchain.swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(ygDevice.device, ygSwapchain.swapchain,
                                     &ygSwapchain.imageCount, NULL));
    ygSwapchain.images =
        YG_MALLOC(ygSwapchain.imageCount * sizeof *ygSwapchain.images);
    ygSwapchain.imageViews =
        YG_MALLOC(ygSwapchain.imageCount * sizeof *ygSwapchain.imageViews);
    VK_CHECK(vkGetSwapchainImagesKHR(ygDevice.device, ygSwapchain.swapchain,
                                     &ygSwapchain.imageCount,
                                     ygSwapchain.images));

    for (uint32_t i = 0; i < ygSwapchain.imageCount; i++) {
        VkImageViewCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = ygSwapchain.images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = ygSwapchain.format,
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

        VK_CHECK(vkCreateImageView(ygDevice.device, &ci, NULL,
                                   &ygSwapchain.imageViews[i]));
    }
}

static void destroySwapchain()
{
    vkDeviceWaitIdle(ygDevice.device);

    // Swapchain images are destroyed in vkDestroySwapchainKHR()
    for (uint32_t i = 0; i < ygSwapchain.imageCount; i++) {
        vkDestroyImageView(ygDevice.device, ygSwapchain.imageViews[i], NULL);
    }

    vkDestroySwapchainKHR(ygDevice.device, ygSwapchain.swapchain, NULL);

    YG_FREE(ygSwapchain.images);
    YG_FREE(ygSwapchain.imageViews);
}

static void createSyncObjects()
{
    VkCommandBufferAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = ygDevice.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = ygSwapchain.framesInFlight,
    };

    ygSwapchain.commandBuffers = YG_MALLOC(ygSwapchain.framesInFlight *
                                           sizeof *ygSwapchain.commandBuffers);
    ygSwapchain.imageAvailableSemaphores =
        YG_MALLOC(ygSwapchain.framesInFlight *
                  sizeof *ygSwapchain.imageAvailableSemaphores);
    ygSwapchain.renderFinishedSemaphores =
        YG_MALLOC(ygSwapchain.framesInFlight *
                  sizeof *ygSwapchain.renderFinishedSemaphores);
    ygSwapchain.inFlightFences = YG_MALLOC(ygSwapchain.framesInFlight *
                                           sizeof *ygSwapchain.inFlightFences);

    VK_CHECK(vkAllocateCommandBuffers(ygDevice.device, &ai,
                                      ygSwapchain.commandBuffers));

    VkSemaphoreCreateInfo sci = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fci = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (uint32_t i = 0; i < ygSwapchain.framesInFlight; i++) {
        VK_CHECK(vkCreateSemaphore(ygDevice.device, &sci, NULL,
                                   &ygSwapchain.imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(ygDevice.device, &sci, NULL,
                                   &ygSwapchain.renderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(ygDevice.device, &fci, NULL,
                               &ygSwapchain.inFlightFences[i]));
    }
}

static void destroySyncObjects()
{
    vkDeviceWaitIdle(ygDevice.device);

    vkFreeCommandBuffers(ygDevice.device, ygDevice.commandPool,
                         ygSwapchain.framesInFlight,
                         ygSwapchain.commandBuffers);

    for (uint32_t i = 0; i < ygSwapchain.framesInFlight; i++) {
        vkDestroySemaphore(ygDevice.device,
                           ygSwapchain.imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(ygDevice.device,
                           ygSwapchain.renderFinishedSemaphores[i], NULL);
        vkDestroyFence(ygDevice.device, ygSwapchain.inFlightFences[i], NULL);
    }

    YG_FREE(ygSwapchain.commandBuffers);
    YG_FREE(ygSwapchain.imageAvailableSemaphores);
    YG_FREE(ygSwapchain.renderFinishedSemaphores);
    YG_FREE(ygSwapchain.inFlightFences);
}

void ygCreateSwapchain(uint32_t framesInFlight,
                       void (*framebufferSizeCallback)(uint32_t*, uint32_t*))
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    if (!framebufferSizeCallback) {
        YG_ERROR("Framebuffer size callback function must be specified");
    }

    ygSwapchain.framesInFlight = framesInFlight;
    ygSwapchain.framebufferSizeCallback = framebufferSizeCallback;

    querySupport();
    createSwapchain();
    createSyncObjects();
}

void ygDestroySwapchain()
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    vkDeviceWaitIdle(ygDevice.device);

    destroySyncObjects();
    destroySwapchain();

    YG_FREE(ygSwapchain.supportDetails.formats);
    YG_FREE(ygSwapchain.supportDetails.presentModes);

    YG_RESET(&ygSwapchain);
}

void ygRecreateSwapchain()
{
    uint32_t width, height;
    ygSwapchain.framebufferSizeCallback(&width, &height);

    YG_DEBUG("Recreating swapchain %" PRIu32 "x%" PRIu32, width, height);

    destroySyncObjects();
    destroySwapchain();

    YG_FREE(ygSwapchain.supportDetails.formats);
    YG_FREE(ygSwapchain.supportDetails.presentModes);

    querySupport();
    createSwapchain();
    createSyncObjects();

    ygSwapchain.recreated = true;
}

VkCommandBuffer ygAcquireNextImage()
{
    // Wait for the current frame to not be in flight
    VK_CHECK(
        vkWaitForFences(ygDevice.device, 1,
                        &ygSwapchain.inFlightFences[ygSwapchain.inFlightIndex],
                        VK_TRUE, UINT64_MAX));
    VK_CHECK(
        vkResetFences(ygDevice.device, 1,
                      &ygSwapchain.inFlightFences[ygSwapchain.inFlightIndex]));

    // Acquire index of next image in the swapchain
    VkResult result = vkAcquireNextImageKHR(
        ygDevice.device, ygSwapchain.swapchain, UINT64_MAX,
        ygSwapchain.imageAvailableSemaphores[ygSwapchain.inFlightIndex], NULL,
        &ygSwapchain.imageIndex);

    // Check if swapchain needs to be reconstructed
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        ygRecreateSwapchain();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        YG_ERROR("Failed to acquire next swapchain image");
    }

    VkCommandBufferBeginInfo bi = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VK_CHECK(vkBeginCommandBuffer(
        ygSwapchain.commandBuffers[ygSwapchain.inFlightIndex], &bi));

    return ygSwapchain.commandBuffers[ygSwapchain.inFlightIndex];
}

void ygPresent(VkCommandBuffer cmd, YgImage* pImage)
{
    if (ygSwapchain.recreated) {
        ygSwapchain.recreated = false;
    }

    // Transition swapchain image for blitting
    ygImageBarrier(cmd, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                   VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                   VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                   VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   ygSwapchain.images[ygSwapchain.imageIndex], NULL);

    // Blit image to current swapchain image
    VkImageSubresourceLayers subresourceLayers = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    int32_t srcWidth = pImage->width;
    int32_t srcHeight = pImage->height;
    int32_t dstWidth = (int32_t)(ygSwapchain.extent.width);
    int32_t dstHeight = (int32_t)(ygSwapchain.extent.height);
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
        .dstImage = ygSwapchain.images[ygSwapchain.imageIndex],
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regionCount = 1,
        .pRegions = &region,
        .filter = VK_FILTER_NEAREST,
    };

    vkCmdBlitImage2(cmd, &blitImageInfo);

    // Transition swapchain image for presenting
    ygImageBarrier(
        cmd, VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_NONE,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        ygSwapchain.images[ygSwapchain.imageIndex], NULL);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags waitStage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo si = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores =
            &ygSwapchain.imageAvailableSemaphores[ygSwapchain.inFlightIndex],
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores =
            &ygSwapchain.renderFinishedSemaphores[ygSwapchain.inFlightIndex],
    };

    VK_CHECK(
        vkQueueSubmit(ygDevice.queue, 1, &si,
                      ygSwapchain.inFlightFences[ygSwapchain.inFlightIndex]));

    VkPresentInfoKHR pi = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores =
            &ygSwapchain.renderFinishedSemaphores[ygSwapchain.inFlightIndex],
        .swapchainCount = 1,
        .pSwapchains = &ygSwapchain.swapchain,
        .pImageIndices = &ygSwapchain.imageIndex,
    };

    VkResult result = vkQueuePresentKHR(ygDevice.queue, &pi);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        ygRecreateSwapchain();
    } else if (result != VK_SUCCESS) {
        YG_ERROR("Failed to present swapchain image");
    }

    ygSwapchain.inFlightIndex =
        (ygSwapchain.inFlightIndex + 1) % ygSwapchain.framesInFlight;
}

void ygCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, YgBuffer* pBuffer)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    *pBuffer = (YgBuffer){
        .usage = usage,
        .properties = properties,
    };

    VkBufferCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VK_CHECK(vkCreateBuffer(ygDevice.device, &ci, NULL, &pBuffer->buffer));

    pBuffer->bufferInfo.buffer = pBuffer->buffer;

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(ygDevice.device, pBuffer->buffer, &memReqs);

    pBuffer->size = memReqs.size;

    VkMemoryAllocateFlagsInfo allocFlagInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
    };

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = ygFindMemoryType(memReqs.memoryTypeBits, properties),
    };

    if (pBuffer->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocInfo.pNext = &allocFlagInfo;
    }

    VK_CHECK(
        vkAllocateMemory(ygDevice.device, &allocInfo, NULL, &pBuffer->memory));

    VK_CHECK(vkBindBufferMemory(ygDevice.device, pBuffer->buffer,
                                pBuffer->memory, 0));

    // Map memory if memory is host coherent
    if (pBuffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        VK_CHECK(vkMapMemory(ygDevice.device, pBuffer->memory, 0, size, 0,
                             &pBuffer->pHostMap));
    }
}

void ygDestroyBuffer(YgBuffer* pBuffer)
{
    vkDeviceWaitIdle(ygDevice.device);

    if (pBuffer->pHostMap) {
        vkUnmapMemory(ygDevice.device, pBuffer->memory);
    }

    vkDestroyBuffer(ygDevice.device, pBuffer->buffer, NULL);
    vkFreeMemory(ygDevice.device, pBuffer->memory, NULL);

    YG_RESET(pBuffer);
}

void ygCopyBufferFromHost(const YgBuffer* pBuffer, const void* pData,
                          VkDeviceSize size, VkDeviceSize offset)
{
    // Check if we need a staging buffer or not
    if (!(pBuffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        // Set up staging buffer
        YgBuffer staging;
        ygCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       &staging);

        // Copy to staging buffer
        ygCopyBufferFromHost(&staging, pData, size, 0);

        // Transfer from staging buffer to this buffer
        VkCommandBuffer cmd = ygCmdBegin();

        VkBufferCopy region = {
            .dstOffset = offset,
            .size = size,
        };
        vkCmdCopyBuffer(cmd, staging.buffer, pBuffer->buffer, 1, &region);

        ygCmdEnd(cmd);

        ygDestroyBuffer(&staging);
    } else {
        // Transfer directly without staging buffer
        char* pOffsettedHostMap = (char*)pBuffer->pHostMap + offset;
        memcpy(pOffsettedHostMap, pData, size);
    }
}

VkWriteDescriptorSet ygGetBufferDescriptor(YgBuffer* pBuffer, uint32_t binding,
                                           VkDeviceSize offset,
                                           VkDeviceSize range)
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
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pBufferInfo = &pBuffer->bufferInfo,
    };
}

void ygCreateImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                   VkSampleCountFlagBits samples, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, YgImage* pImage)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    *pImage = (YgImage){
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

    VK_CHECK(vkCreateImage(ygDevice.device, &ci, NULL, &pImage->image));

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(ygDevice.device, pImage->image, &memReqs);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = ygFindMemoryType(memReqs.memoryTypeBits, properties),
    };

    VK_CHECK(
        vkAllocateMemory(ygDevice.device, &allocInfo, NULL, &pImage->memory));

    VK_CHECK(
        vkBindImageMemory(ygDevice.device, pImage->image, pImage->memory, 0));
}

void ygDestroyImage(YgImage* pImage)
{
    vkDeviceWaitIdle(ygDevice.device);

    vkFreeMemory(ygDevice.device, pImage->memory, NULL);
    vkDestroyImageView(ygDevice.device, pImage->imageView, NULL);
    vkDestroyImage(ygDevice.device, pImage->image, NULL);

    YG_RESET(pImage);
}

void ygCreateImageView(YgImage* pImage, VkImageAspectFlags aspectFlags)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
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

    VK_CHECK(vkCreateImageView(ygDevice.device, &ci, NULL, &pImage->imageView));
}

void ygCreateSampler(VkFilter magFilter, VkFilter minFilter,
                     VkSamplerMipmapMode mipmapMode,
                     VkSamplerAddressMode addressModeU,
                     VkSamplerAddressMode addressModeV,
                     VkSamplerAddressMode addressModeW, YgSampler* pSampler)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    VkSamplerCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = magFilter,
        .minFilter = minFilter,
        .mipmapMode = mipmapMode,
        .addressModeU = addressModeU,
        .addressModeV = addressModeV,
        .addressModeW = addressModeW,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy =
            ygDevice.properties.physicalDevice.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 1000.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VK_CHECK(vkCreateSampler(ygDevice.device, &ci, NULL, &pSampler->sampler));
}

void ygDestroySampler(YgSampler* pSampler)
{
    vkDeviceWaitIdle(ygDevice.device);

    vkDestroySampler(ygDevice.device, pSampler->sampler, NULL);

    YG_RESET(pSampler);
}

static void generateMipmaps(YgTexture* pTexture)
{
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(ygDevice.physicalDevice,
                                        pTexture->image.format, &props);

    if (!(props.optimalTilingFeatures &
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        YG_ERROR("Texture image format does not support linear blitting");
    }

    VkCommandBuffer cmd = ygCmdBegin();

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

        ygImageBarrier(
            cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pTexture->image.image,
            &subresourceRange);

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
            .dstOffsets = {{0, 0, 0},
                           {mipWidth > 1 ? mipWidth / 2 : 1,
                            mipHeight > 1 ? mipHeight / 2 : 1, 1}},
        };

        VkBlitImageInfo2 blitImageInfo = {
            .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            .srcImage = pTexture->image.image,
            .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstImage = pTexture->image.image,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &region,
            .filter = VK_FILTER_NEAREST,
        };

        vkCmdBlitImage2(cmd, &blitImageInfo);

        ygImageBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
                       VK_ACCESS_2_TRANSFER_READ_BIT,
                       VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                       VK_ACCESS_2_SHADER_READ_BIT,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       pTexture->image.image, &subresourceRange);

        if (mipWidth > 1) {
            mipWidth /= 2;
        }
        if (mipHeight > 1) {
            mipHeight /= 2;
        }
    }

    subresourceRange.baseMipLevel = pTexture->image.mipLevels - 1;

    ygImageBarrier(
        cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
        VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, pTexture->image.image,
        &subresourceRange);

    ygCmdEnd(cmd);
}

static void createTexture(YgTexture* pTexture, enum YgTextureType type,
                          VkFormat format, const void* pData, uint32_t width,
                          uint32_t height, uint32_t channels, bool mipmaps)
{
    uint32_t mipLevels = 1;
    if (mipmaps) {
        mipLevels = (uint32_t)floor(log2(YG_MAX(width, height))) + 1;
    }

    ygCreateImage(width, height, mipLevels, VK_SAMPLE_COUNT_1_BIT, format,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                      VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pTexture->image);

    if (pData) {
        VkDeviceSize size = width * height * channels;

        YgBuffer staging;
        ygCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       &staging);

        ygCopyBufferFromHost(&staging, pData, size, 0);

        VkCommandBuffer cmd = ygCmdBegin();

        ygImageBarrier(
            cmd, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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

        vkCmdCopyBufferToImage(cmd, staging.buffer, pTexture->image.image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &region);

        ygCmdEnd(cmd);

        if (mipmaps) {
            generateMipmaps(pTexture);
        } else {
            cmd = ygCmdBegin();
            ygImageBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                           VK_ACCESS_2_TRANSFER_WRITE_BIT,
                           VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                           VK_ACCESS_2_SHADER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           pTexture->image.image, NULL);
            ygCmdEnd(cmd);
        }

        ygDestroyBuffer(&staging);
    }

    ygCreateImageView(&pTexture->image, VK_IMAGE_ASPECT_COLOR_BIT);

    pTexture->imageInfo = (VkDescriptorImageInfo){
        .sampler = NULL,
        .imageView = pTexture->image.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}

void ygCreateTexture(enum YgTextureType type, VkFormat format,
                     const void* pData, uint32_t width, uint32_t height,
                     uint32_t channels, bool generateMipmaps,
                     YgTexture* pTexture)
{
    createTexture(pTexture, type, format, pData, width, height, channels,
                  generateMipmaps);
}

#ifdef YGGDRASIL_STBI
void ygCreateTextureFromFile(enum YgTextureType type, VkFormat format,
                             const char* pPath, bool generateMipmaps,
                             YgTexture* pTexture)
{
    YG_INFO("Loading texture from %s", pPath);

    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;

    stbi_uc* pData =
        stbi_load(pPath, &width, &height, &channels, STBI_rgb_alpha);
    channels = STBI_rgb_alpha;

    if (!pData) {
        YG_ERROR("Failed to load texture.");
        return;
    }

    createTexture(pTexture, type, format, pData, width, height, channels,
                  generateMipmaps);

    stbi_image_free(pData);
}
#endif

void ygDestroyTexture(YgTexture* pTexture)
{
    ygDestroyImage(&pTexture->image);
}

void ygSetTextureSampler(YgTexture* pTexture, const YgSampler* pSampler)
{
    pTexture->imageInfo.sampler = pSampler->sampler;
}

VkWriteDescriptorSet ygGetTextureDescriptor(const YgTexture* pTexture,
                                            uint32_t binding)
{
    return (VkWriteDescriptorSet){
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &pTexture->imageInfo,
    };
}

static createPass(YgPass* pPass, YgImage* pColorAttachments,
                  uint32_t colorAttachmentCount, YgImage* pDepthAttachment,
                  YgImage* pResolveAttachment)
{
    *pPass = (YgPass){
        .pRenderingAttachmentInfos =
            YG_MALLOC(colorAttachmentCount * sizeof(VkRenderingAttachmentInfo)),
        .pFormats = YG_MALLOC(colorAttachmentCount * sizeof(VkFormat)),
        .pColorAttachments = pColorAttachments,
        .colorAttachmentCount = colorAttachmentCount,
        .pDepthAttachment = pDepthAttachment,
        .pResolveAttachment = pResolveAttachment,
    };

    for (uint32_t i = 0; i < colorAttachmentCount; i++) {
        pPass->pRenderingAttachmentInfos[i] = (VkRenderingAttachmentInfo){
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = pPass->pColorAttachments[i].imageView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = pPass->pResolveAttachment
                               ? VK_RESOLVE_MODE_AVERAGE_BIT
                               : VK_RESOLVE_MODE_NONE,
            .resolveImageView = pPass->pResolveAttachment
                                    ? pPass->pResolveAttachment->imageView
                                    : NULL,
            .resolveImageLayout = pPass->pResolveAttachment
                                      ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                      : VK_IMAGE_LAYOUT_UNDEFINED,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };
        pPass->pFormats[i] = pPass->pColorAttachments[i].format;
    }

    pPass->pipelineRenderingCreateInfo = (VkPipelineRenderingCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = colorAttachmentCount,
        .pColorAttachmentFormats = pPass->pFormats,
        .depthAttachmentFormat = pDepthAttachment->format,
    };
}

void ygCreatePass(YgImage* pColorAttachments, uint32_t colorAttachmentCount,
                  YgImage* pDepthAttachment, YgImage* pResolveAttachment,
                  YgPass* pPass)
{
    createPass(pPass, pColorAttachments, colorAttachmentCount, pDepthAttachment,
               pResolveAttachment);
}

void ygDestroyPass(YgPass* pPass)
{
    YG_FREE(pPass->pRenderingAttachmentInfos);
    YG_FREE(pPass->pFormats);

    YG_RESET(pPass);
}

void ygRecreatePass(YgPass* pPass, YgImage* pColorAttachments,
                    uint32_t colorAttachmentCount, YgImage* pDepthAttachment,
                    YgImage* pResolveAttachment)
{
    YG_FREE(pPass->pRenderingAttachmentInfos);
    YG_FREE(pPass->pFormats);
    createPass(pPass, pColorAttachments, colorAttachmentCount, pDepthAttachment,
               pResolveAttachment);
}

void ygBeginPass(const YgPass* pPass, VkCommandBuffer cmd,
                 VkClearValue clearValue, VkAttachmentLoadOp loadOp)
{
    for (uint32_t i = 0; i < pPass->colorAttachmentCount; i++) {
        pPass->pRenderingAttachmentInfos[i].clearValue = clearValue;
        pPass->pRenderingAttachmentInfos[i].loadOp = loadOp;
    }

    VkRenderingAttachmentInfo depthAttachmentInfo;
    if (pPass->pDepthAttachment) {
        depthAttachmentInfo = (VkRenderingAttachmentInfo){
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = pPass->pDepthAttachment->imageView,
            .imageLayout =
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearValue,
        };
    }

    VkExtent2D extent = {
        .width = pPass->pColorAttachments[0].width,
        .height = pPass->pColorAttachments[0].height,
    };

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {.offset = {0, 0}, .extent = extent},
        .layerCount = 1,
        .colorAttachmentCount = pPass->colorAttachmentCount,
        .pColorAttachments = pPass->pRenderingAttachmentInfos,
        .pDepthAttachment =
            pPass->pDepthAttachment ? &depthAttachmentInfo : NULL,
        .pStencilAttachment = NULL,
    };

    vkCmdBeginRendering(cmd, &renderingInfo);
}

void ygEndPass(const YgPass* pPass, VkCommandBuffer cmd)
{
    vkCmdEndRendering(cmd);
}


#endif

#ifdef __cplusplus
}
#endif
