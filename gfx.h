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

// Define GFX_USE_STB_IMAGE_WRITE if stb_image_write.h is available. This allows
// for saving images to file.
#ifdef GFX_USE_STB_IMAGE_WRITE
#include "stb_image_write.h"
#endif

// Depth goes from 0 at the near plane to 1 at the far plane, so depth is
// cleared to 1.0 and the smaller depth wins. Define GFX_REVERSE_Z before
// including gfx.h to reverse this.
#ifdef GFX_REVERSE_Z
#define GFX_DEPTH_CLEAR 0.0f
#define GFX_DEPTH_COMPARE_OP VK_COMPARE_OP_GREATER
#else
#define GFX_DEPTH_CLEAR 1.0f
#define GFX_DEPTH_COMPARE_OP VK_COMPARE_OP_LESS
#endif

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if GFX_LINUX
#include <signal.h>
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

static const struct GfxVulkanResult {
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
#define GFX_INFO(fmt, ...)                                                                                             \
    do {                                                                                                               \
        fprintf(stdout, "INFO: " fmt "\n", ##__VA_ARGS__);                                                             \
    } while (0)

#ifdef NDEBUG
#define GFX_DEBUG(fmt, ...)                                                                                            \
    do {                                                                                                               \
    } while (0)
#else
// Log a debug message. Only visible if NDEBUG is not defined.
#define GFX_DEBUG(fmt, ...)                                                                                            \
    do {                                                                                                               \
        fprintf(stdout, "\x1B[1;92mDEBUG: \x1B[0m" fmt "\n", ##__VA_ARGS__);                                           \
    } while (0)
#endif

// Log a warning message
#define GFX_WARNING(fmt, ...)                                                                                          \
    do {                                                                                                               \
        fprintf(stderr, "\x1B[1;93mWARNING: \x1B[0m%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);              \
    } while (0)

#ifdef NDEBUG
// Break into the debugger, or abort if there is none to break into
#define GFX_BREAK() abort()
#elif GFX_WINDOWS
#define GFX_BREAK() __debugbreak()
#elif GFX_LINUX
#define GFX_BREAK() raise(SIGTRAP)
#endif

// Log an error message. Will cause a breakpoint (SIGTRAP on Linux) if NDEBUG is
// not defined, and abort otherwise.
#define GFX_ERROR(fmt, ...)                                                                                            \
    do {                                                                                                               \
        fprintf(stderr, "\x1B[1;91mERROR: \x1B[0m%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);                \
        GFX_BREAK();                                                                                                   \
    } while (0)

// Get the string representation of a VkResult. Returns "VK_RESULT_UNKNOWN" for
// codes that are not in the table.
static inline const char* gfxResultString(VkResult result)
{
    for (uint32_t i = 0; i < GFX_ARRAY_LEN(gfxVulkanResults); i++) {
        if (result == gfxVulkanResults[i].result) {
            return gfxVulkanResults[i].string;
        }
    }
    return "VK_RESULT_UNKNOWN";
}

// Check a Vulkan function call for errors. Only negative results are errors;
// positive results (VK_INCOMPLETE, VK_SUBOPTIMAL_KHR, ...) are successes.
#define VK_CHECK(x)                                                                                                    \
    do {                                                                                                               \
        VkResult gfxCheckedResult = (x);                                                                               \
        if (gfxCheckedResult < 0) {                                                                                    \
            GFX_ERROR("%s (%d)", gfxResultString(gfxCheckedResult), (int)gfxCheckedResult);                            \
        }                                                                                                              \
    } while (0)


// GFX type definitions //

// Properties of the chosen physical device
typedef struct GfxDeviceProperties {
    VkPhysicalDeviceProperties physicalDevice;
    VkPhysicalDeviceMemoryProperties memory;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipeline;
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructure;
} GfxDeviceProperties;

// Description of a physical device, as listed by gfxEnumerateDevices(). A
// device is supported if it has Vulkan 1.3, shader objects and a queue that
// gfxCreateDevice() can use, which must also be able to present to the
// surface if one was given.
typedef struct GfxDeviceInfo {
    char name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    uint8_t uuid[VK_UUID_SIZE]; // Same across runs and driver versions
    VkPhysicalDeviceType type;
    uint32_t apiVersion;
    bool supported;
} GfxDeviceInfo;

// Device level function pointers for extensions that the Vulkan loader does not
// export directly. Resolved once by gfxCreateDevice() instead of on every call.
// Call them through gfxDevice.fn, for instance:
//     gfxDevice.fn.vkCmdSetPolygonModeEXT(cmd, VK_POLYGON_MODE_LINE);
typedef struct GfxDeviceFunctions {
    PFN_vkCreateShadersEXT vkCreateShadersEXT;
    PFN_vkDestroyShaderEXT vkDestroyShaderEXT;
    PFN_vkCmdBindShadersEXT vkCmdBindShadersEXT;
    PFN_vkCmdSetVertexInputEXT vkCmdSetVertexInputEXT;
    PFN_vkCmdSetRasterizationSamplesEXT vkCmdSetRasterizationSamplesEXT;
    PFN_vkCmdSetSampleMaskEXT vkCmdSetSampleMaskEXT;
    PFN_vkCmdSetAlphaToCoverageEnableEXT vkCmdSetAlphaToCoverageEnableEXT;
    PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT;
    PFN_vkCmdSetLogicOpEnableEXT vkCmdSetLogicOpEnableEXT;
    PFN_vkCmdSetColorBlendEnableEXT vkCmdSetColorBlendEnableEXT;
    PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT;
    PFN_vkCmdSetColorBlendEquationEXT vkCmdSetColorBlendEquationEXT;
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
    PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
} GfxDeviceFunctions;

// Instance holds the Vulkan instance that the device and surface are created
// from. The GFX instance is monolithic and is setup through
// gfxCreateInstance(). Create the window surface from gfxInstance.instance
// before creating the device. Release resources with gfxDestroyInstance().
typedef struct GfxInstance {
    VkInstance instance;
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    uint32_t apiVersion;
    bool debugUtils; // VK_EXT_debug_utils is enabled
} GfxInstance;

// Device contains a Vulkan context for rendering. The GFX device is
// monolithic and is setup through gfxCreateDevice(), after the instance has
// been created. Release resources with gfxDestroyDevice().
typedef struct GfxDevice {
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    GfxDeviceProperties properties;
    GfxDeviceFunctions fn;
    VkCommandPool commandPool;
    VkQueue queue;
    uint32_t queueFamilyIndex;
    VkShaderStageFlags graphicsShaderStages;
    bool accelerationStructures; // VK_KHR_acceleration_structure is enabled
    bool rayTracingPipelines;    // VK_KHR_ray_tracing_pipeline is enabled
    uint64_t timestampMask;      // Valid bits of a timestamp, 0 if the queue has no timestamps
    uint64_t frame;
    uint64_t completedFrames;
    uint64_t resourceGeneration;
    bool vsync;
    bool samplerAnisotropy;
} GfxDevice;

// Swapchain abstracts the handling of swapchain images and frames in flight.
// The GFX swapchain is monolithic and is setup through
// gfxCreateSwapchain(). Use gfxAcquireNextImage() and gfxPresent() to acquire and
// present images from the swapchain respectively. The swapchain can be
// recreated with gfxRecreateSwapchain() if the framebuffer size changed. The
// framebufferSizeCallback() function pointer will be used to retrieve the new
// framebuffer size. The recreated flag is set when the swapchain has been
// recreated and stays set until the next gfxAcquireNextImage(), so that
// attachments can be resized at the top of the frame loop. Release resources
// with gfxDestroySwapchain().
typedef struct GfxSwapchain {
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;
    bool recreated;

    void (*framebufferSizeCallback)(uint32_t*, uint32_t*);

    struct GfxSwapchainSupportDetails {
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

// Size of the blocks of device memory that buffers and images are placed in.
// Heaps smaller than eight blocks get blocks of an eighth of the heap, and
// resources larger than half a block, or that the driver wants alone, get
// memory of their own.
#ifndef GFX_MEMORY_BLOCK_SIZE
#define GFX_MEMORY_BLOCK_SIZE (64ull * 1024 * 1024)
#endif

// Buffer abstracts a Vulkan buffer and its memory. Buffers are created with
// gfxCreateBuffer(), and placed in blocks of device memory shared with other
// resources, see GFX_MEMORY_BLOCK_SIZE, from memoryOffset on. Copy data from
// host to the device allocated buffer with gfxCopyBufferFromHost(). A buffer
// with host coherent memory will always be mapped on pHostMap. Copies to
// buffers with memory that is not host coherent go through staging memory and
// are batched with other uploads, see gfxFlushUploads(). Data that changes
// every frame belongs in a per-frame buffer, created with
// gfxCreatePerFrameBuffer(), which holds one copy per frame in flight. Write
// this frame's copy through gfxGetPerFrameData(); shaders the buffer is
// attached to read the copy of the frame they are recorded in. Release
// resources with gfxDestroyBuffer().
typedef struct GfxBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize memoryOffset; // Where the buffer starts in memory
    VkDeviceSize memorySize;   // Bytes of memory taken by the buffer
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
    VkDeviceSize size;
    void* pHostMap;
    uint32_t frameCount;      // Number of per-frame copies, 0 for an ordinary buffer
    VkDeviceSize frameSize;   // Size of one per-frame copy
    VkDeviceSize frameStride; // Distance between per-frame copies
} GfxBuffer;

// Image abstracts a Vulkan image, image view and its memory. Use
// gfxCreateImage() to create a new image and gfxCreateImageView() to create an
// image view for a created image. Like buffers, images are placed in blocks of
// device memory shared with other resources. Release resources with
// gfxDestroyImage().
typedef struct GfxImage {
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
    VkDeviceSize memoryOffset; // Where the image starts in memory
    VkDeviceSize memorySize;   // Bytes of memory taken by the image
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t arrayLayers;
    uint32_t mipLevels;
    VkFormat format;
    VkImageTiling tiling;
    VkImageType imageType;
    VkImageViewType viewType;
    VkSampleCountFlagBits samples;
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
// are automatically generated if specified. A sampler is created with the
// texture; use the gfxSetTexture*() functions to reconfigure it. Attach it to
// a shader with gfxSetShaderTexture(). Release resources with
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

// Attachment sets keeps track of color and depth attachments and prepares for
// dynamic rendering. Create a new pass attachment description with
// gfxCreateAttachment(). Before rendering with the attachment call
// gfxCmdBeginRendering() and call gfxCmdEndRendering() when done. If the attachment
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

// Triangle geometry for a bottom level acceleration structure. Vertex and
// index buffers need VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT and
// VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR.
// Vertices are VK_FORMAT_R32G32B32_SFLOAT unless another format is given, and
// indices are 32 bit. Without an index buffer every three vertices form a
// triangle. Set flags to VK_GEOMETRY_OPAQUE_BIT_KHR for geometry that should
// not invoke any-hit shaders.
typedef struct GfxGeometryDesc {
    const GfxBuffer* pVertexBuffer;
    VkDeviceSize vertexOffset;
    VkDeviceSize vertexStride;
    uint32_t vertexCount;
    VkFormat vertexFormat;
    const GfxBuffer* pIndexBuffer;
    VkDeviceSize indexOffset;
    uint32_t indexCount;
    VkGeometryFlagsKHR flags;
} GfxGeometryDesc;

// Acceleration structure for ray tracing. A bottom level acceleration
// structure (BLAS) holds geometry and is created with gfxCreateBLAS(). A top
// level acceleration structure (TLAS) holds instances of BLASes and is created
// with gfxCreateTLAS(). Instances can be changed every frame with
// gfxCmdUpdateTLAS(), up to the number the TLAS was created with. Attach a
// TLAS to a shader with gfxSetShaderAccelerationStructure(). Release resources
// with gfxDestroyAccelerationStructure().
typedef struct GfxAccelerationStructure {
    VkAccelerationStructureKHR accelerationStructure;
    VkAccelerationStructureTypeKHR type;
    VkBuildAccelerationStructureFlagsKHR flags;
    VkDeviceAddress address;
    GfxBuffer buffer;
    GfxBuffer scratch;   // Kept by a TLAS for updates
    GfxBuffer instances; // TLAS only
    uint32_t instanceCapacity;
    uint32_t primitiveCount; // Triangles of a BLAS, instances of a TLAS
} GfxAccelerationStructure;

// Describes one stage of a shader. Give either a path or SPIR-V byte code.
// Paths ending in .spv are loaded as SPIR-V, anything else is compiled as GLSL
// and may use #include with paths relative to the including file. The stage
// can be left as 0, in which case it is inferred from the file extension
// (.vert, .tesc, .tese, .geom, .frag, .comp, .task, .mesh, .rgen, .rint,
// .rahit, .rchit, .rmiss or .rcall, optionally followed by .glsl) or, for
// SPIR-V, from the entry point.
typedef struct GfxShaderStageDesc {
    const char* pPath;
    const void* pCode;
    size_t codeSize;
    VkShaderStageFlagBits stage;
    const VkSpecializationInfo* pSpecializationInfo;
} GfxShaderStageDesc;

// One stage of a shader.
typedef struct GfxShaderStage {
    VkShaderStageFlagBits stage;
    VkShaderEXT shader;
    char* pPath;
    uint32_t* pCode;
    size_t codeSize;
    VkSpecializationInfo* pSpecializationInfo;
} GfxShaderStage;

// A descriptor binding as reflected from the shader, merged over all stages,
// together with the resource attached to it. Resources are attached by pointer
// and resolved when the shader's resources are bound, so the attached objects
// must stay at the same address for as long as they are attached.
typedef struct GfxShaderBinding {
    char* pName;      // Variable name, empty for blocks declared without an instance name
    char* pBlockName; // Block type name, NULL for resources that are not blocks
    uint32_t set;
    uint32_t binding;
    VkDescriptorType type;
    uint32_t count; // Number of descriptors, greater than one for arrays
    VkShaderStageFlags stages;
    uint32_t infoIndex; // Where the descriptor infos of the binding start

    const GfxBuffer* pBuffer;
    VkDeviceSize offset;
    VkDeviceSize range;
    const GfxTexture* pTextures; // count textures
    const GfxImage* pImage;
    VkImageLayout imageLayout;
    VkSampler sampler; // Combined with pImage
    const GfxAccelerationStructure* pAccelerationStructure;

    bool reported; // Whether a missing resource has been reported
} GfxShaderBinding;

// A descriptor set retired from use, waiting for the frame that last used it
// to finish before it can be written again.
typedef struct GfxRetiredDescriptorSet {
    VkDescriptorSet set;
    uint64_t frame;
} GfxRetiredDescriptorSet;

// One descriptor set of a shader. Bindings and descriptor infos of the set are
// contiguous ranges in the shader's arrays.
typedef struct GfxShaderSet {
    VkDescriptorSetLayout layout;
    uint32_t firstBinding;
    uint32_t bindingCount;
    uint32_t firstInfo;
    uint32_t infoCount;

    VkDescriptorSet current; // Set holding the descriptors last written
    uint64_t generation;     // gfxDevice.resourceGeneration when current was written
    uint32_t retiredCount;
    uint32_t retiredCapacity;
    GfxRetiredDescriptorSet* pRetired;
} GfxShaderSet;

// Shader abstracts the handling of shaders, descriptors and layouts and builds
// on top of Vulkan's shader objects. Create a new shader from one or more
// stages with gfxCreateShader(). Graphics stages are linked together. The
// descriptor set layouts, pipeline layout and push constant range are all
// reflected from the SPIR-V.
//
// Resources are attached by the name they have in the shader, either the
// variable name or, for blocks declared without an instance name, the block
// type name. Use gfxSetShaderBuffer(), gfxSetShaderTexture(),
// gfxSetShaderTextureArray() and gfxSetShaderImage(). Bind the shader with
// gfxCmdBindShader() and its resources with gfxCmdBindResources(). A
// descriptor set is only rewritten when what is attached to it changed; a set
// that a frame in flight may still read is never written, a recycled one is
// taken instead. Recycling follows the frames presented with gfxPresent().
//
// Shaders compiled from GLSL can be recompiled with gfxReloadShader(), which
// keeps the attached resources. Release resources with gfxDestroyShader().
typedef struct GfxShader {
    uint32_t stageCount;
    GfxShaderStage* pStages;
    VkShaderStageFlags stages;
    uint32_t version; // Advanced by gfxReloadShader()

    uint32_t setCount;
    GfxShaderSet* pSets;
    VkPipelineLayout pipelineLayout;
    VkPushConstantRange pushConstantRange; // Size is 0 if there are no push constants
    uint32_t localSize[3];                 // Workgroup size of a compute shader

    uint32_t bindingCount;
    GfxShaderBinding* pBindings; // Sorted by set and binding

    // Descriptor infos, indexed by GfxShaderBinding::infoIndex. The written
    // infos are what the current sets hold, the resolved ones are scratch.
    uint32_t infoCount;
    VkDescriptorBufferInfo* pWrittenBufferInfos;
    VkDescriptorImageInfo* pWrittenImageInfos;
    VkDescriptorBufferInfo* pResolvedBufferInfos;
    VkDescriptorImageInfo* pResolvedImageInfos;
    VkAccelerationStructureKHR* pWrittenAccelerationStructures;
    VkAccelerationStructureKHR* pResolvedAccelerationStructures;
    VkWriteDescriptorSet* pWrites;                                              // Scratch, one per binding
    VkWriteDescriptorSetAccelerationStructureKHR* pAccelerationStructureWrites; // Scratch, one per binding

    uint32_t poolCount;
    VkDescriptorPool* pPools;
    uint32_t poolSizeCount;
    VkDescriptorPoolSize* pPoolSizes;
} GfxShader;

// Ray tracing pipeline built from a shader with ray tracing stages, together
// with its shader binding table. Shader groups follow the order of the
// shader's stages: every ray generation, miss and callable stage is a group of
// its own, in the order given, and consecutive closest hit, any hit and
// intersection stages form one hit group. A new hit group starts when a stage
// of a kind the current group already has comes along, so [closest hit, any
// hit, closest hit] gives two hit groups. Create the pipeline with
// gfxCreateRayTracingPipeline(), bind it with gfxCmdBindRayTracingPipeline()
// and the shader's resources with gfxCmdBindResources(), then trace with
// gfxCmdTraceRays(). The pipeline is rebuilt on bind if its shader was
// reloaded. Release resources with gfxDestroyRayTracingPipeline().
typedef struct GfxRayTracingPipeline {
    VkPipeline pipeline;
    GfxShader* pShader;
    uint32_t shaderVersion;
    uint32_t maxRecursionDepth;
    GfxBuffer shaderBindingTable;
    VkStridedDeviceAddressRegionKHR raygenRegion;
    VkStridedDeviceAddressRegionKHR missRegion;
    VkStridedDeviceAddressRegionKHR hitRegion;
    VkStridedDeviceAddressRegionKHR callableRegion;
} GfxRayTracingPipeline;

// Most resources a render graph pass can read and write
#ifndef GFX_MAX_PASS_RESOURCES
#define GFX_MAX_PASS_RESOURCES 8
#endif

// Describes an image owned by a render graph. The usage flags decide how passes
// access it: a graphics pass writes an image with attachment usage as an
// attachment and any other image as a storage image, and a pass reads an image
// as sampled if it has sampled usage and as a storage image otherwise. Leave
// width and height as 0 to follow the size of the swapchain.
//
// Multisampling is set with samples, where 0 means single-sampled. Graphics
// passes render a multisampled color image into a multisampled image the graph
// keeps on the side, and resolve it by averaging into the image itself at the
// end of the pass, so it needs a float or normalized format and attachment
// usage. Everything else, other passes, gfxGetGraphImage() and gfxPresent(),
// sees the resolved image. A multisampled depth image is not resolved. The
// attachments of a pass must all have the same sample count.
typedef struct GfxGraphImageDesc {
    const char* pName;
    VkFormat format;
    VkImageUsageFlags usage;
    uint32_t width;
    uint32_t height;
    VkSampleCountFlagBits samples;
    VkClearColorValue clearColor; // Color attachments are cleared to this, depth to GFX_DEPTH_CLEAR
} GfxGraphImageDesc;

// A graphics pass renders to the color and depth attachments among its outputs,
// which can be a depth image alone, as for a shadow map. A compute pass
// dispatches or traces rays and writes its outputs as storage.
enum GfxPassType {
    GFX_PASS_GRAPHICS,
    GFX_PASS_COMPUTE,
};

// Records the commands of a pass. A graphics pass is called between the
// beginning and end of rendering, with the attachments it renders to; a
// compute pass is called with NULL.
typedef void (*GfxPassFunction)(VkCommandBuffer cmd, const GfxAttachment* pAttachment, void* pUserData);

// Describes a pass of a render graph by name, the resources it reads and
// writes, and the function recording it. The color attachments of a graphics
// pass are its color outputs, in the order they are listed.
typedef struct GfxPassDesc {
    const char* pName;
    enum GfxPassType type;
    const char* inputs[GFX_MAX_PASS_RESOURCES];
    const char* outputs[GFX_MAX_PASS_RESOURCES];
    GfxPassFunction function;
    void* pUserData;
} GfxPassDesc;

typedef struct GfxGraphResource {
    char* pName;
    GfxImage image;           // Owned by the graph, for images
    GfxImage multisampled;    // Rendered to and resolved into image, for multisampled color images
    const GfxBuffer* pBuffer; // Owned by the application, for buffers
    GfxGraphImageDesc desc;
    VkImageLayout layout; // Layout the image is in where recording has got to
    bool written;         // Whether the last access was a write
} GfxGraphResource;

typedef struct GfxGraphPass {
    char* pName;
    enum GfxPassType type;
    char* inputNames[GFX_MAX_PASS_RESOURCES];
    char* outputNames[GFX_MAX_PASS_RESOURCES];
    uint32_t inputCount;
    uint32_t outputCount;
    uint32_t inputs[GFX_MAX_PASS_RESOURCES]; // Resource indices, resolved when compiled
    uint32_t outputs[GFX_MAX_PASS_RESOURCES];
    GfxPassFunction function;
    void* pUserData;
    GfxAttachment attachment; // Graphics passes only
    GfxImage* pColorImages;   // Copies of the color attachments, as GfxAttachment needs them adjacent
    GfxImage depthImage;
    bool hasDepth;
} GfxGraphPass;

// Render graph that owns the images a frame is built from, and records the
// passes that read and write them in the order the data flows between them.
// Passes can be added in any order; every layout transition and barrier
// between passes, and between frames, is recorded by the graph, and graphics
// passes are rendered without any attachment handling by the application.
// Images that follow the swapchain size are recreated when it changes, and
// since resources are attached to shaders by pointer, shaders pick the new
// images up on their own.
//
// Create a graph with gfxCreateRenderGraph() and add images with
// gfxAddGraphImage(), buffers with gfxAddGraphBuffer() and passes with
// gfxAddGraphPass(). Mark the image that is presented with
// gfxSetGraphOutput(), and record a frame with gfxCmdExecuteRenderGraph(). A
// resource can only be written by one pass. Release resources with
// gfxDestroyRenderGraph().
typedef struct GfxRenderGraph {
    uint32_t resourceCount;
    GfxGraphResource** ppResources; // Allocated one by one, so that images stay at the same address
    uint32_t passCount;
    GfxGraphPass* pPasses;
    uint32_t* pOrder; // Pass indices in execution order
    uint32_t output;  // Index of the presented image, UINT32_MAX if none
    bool compiled;
} GfxRenderGraph;

// Most GPU timers a frame can have, including the one for the whole frame
#ifndef GFX_MAX_TIMERS
#define GFX_MAX_TIMERS 64
#endif

#define GFX_MAX_TIMER_NAME_LENGTH 64

// GPU time of a region of a frame, measured with gfxCmdBeginTimer() and
// gfxCmdEndTimer(). Every frame has a timer named "Frame" around all of it, from
// gfxAcquireNextImage() to gfxPresent(), and render graphs time each pass.
// Results are read back with gfxGetTimerResults() when the frame has finished.
typedef struct GfxTimerResult {
    char name[GFX_MAX_TIMER_NAME_LENGTH];
    uint32_t depth; // Number of timers it is nested in
    double milliseconds;
} GfxTimerResult;


// Monolithic global variables //

extern GfxInstance gfxInstance;
extern GfxDevice gfxDevice;
extern GfxSwapchain gfxSwapchain;

// Load a device level function pointer as Xvk...(). The entry points used by
// GFX itself are already cached in gfxDevice.fn; use this for any others, for
// instance:
//     VK_LOAD(vkCmdDrawMeshTasksEXT);
//     XvkCmdDrawMeshTasksEXT(cmd, ...);
// Prefer hoisting the load out of hot loops, since it is a loader lookup.
#define VK_LOAD(func_name)                                                                                             \
    PFN_##func_name X##func_name = (PFN_##func_name)vkGetDeviceProcAddr(gfxDevice.device, #func_name)


// Function declarations //

/// <summary>
/// Create a new instance. Handle is internally managed and accessible through
/// gfxInstance.
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
/// List the physical devices of the instance, for instance to let the user
/// pick one. The index of a device in the list is what gfxCreateDevice() takes.
/// Indices can change between runs, so remember a choice by its UUID. Works
/// like Vulkan's enumerate functions: with pDevices NULL the number of devices
/// is returned in pDeviceCount, otherwise at most *pDeviceCount devices are
/// written and pDeviceCount is set to the number written.
/// </summary>
/// <param name="surface">Surface the device must be able to present to for it to be supported, can be
/// VK_NULL_HANDLE</param>
/// <param name="pDeviceCount">Number of devices</param>
/// <param name="pDevices">Where the device descriptions will be stored, can be NULL</param>
void gfxEnumerateDevices(VkSurfaceKHR surface, uint32_t* pDeviceCount, GfxDeviceInfo* pDevices);

// Pass to gfxCreateDevice() to pick the first supported discrete GPU, or the
// first supported device if there is no supported discrete GPU. Supported is
// as reported by gfxEnumerateDevices() for the surface.
#define GFX_DEFAULT_DEVICE UINT32_MAX

/// <summary>
/// Create a new device. Handle is internally managed and accessible through
/// gfxDevice. The available devices are listed with their indices when it is
/// created, and can be listed beforehand with gfxEnumerateDevices().
///
/// The extensions and features that yggdrasil uses are always enabled, and
/// ray tracing and anisotropic filtering are enabled when the device supports
/// them. The extensions and features passed here are enabled in addition. The
/// features yggdrasil needs are switched on in the passed structures when they
/// are of the same type, so core features must be passed in
/// VkPhysicalDeviceVulkan1xFeatures and not in the structures of the
/// extensions that were promoted to core.
/// </summary>
/// <param name="physicalDeviceIndex">Index of the physical device to use, or GFX_DEFAULT_DEVICE</param>
/// <param name="deviceExtensionCount">Number of extra device extensions</param>
/// <param name="ppDeviceExtensions">List of extra device extensions, can be NULL</param>
/// <param name="features">Extra features, can be NULL</param>
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
/// Wait for the fence of the current frame in flight to be signaled.
/// Call this before accessing any shared resources.
/// </summary>
void gfxWaitForFence();

/// <summary>
/// Acquire a new image from the swapchain. This call will block until an image
/// is available. If the swapchain was out of date it is recreated and
/// VK_NULL_HANDLE is returned; the caller must then skip the frame and check
/// gfxSwapchain.recreated before acquiring again.
/// </summary>
/// <returns>Command buffer to use to render to the new image, or VK_NULL_HANDLE if no image was acquired</returns>
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
/// Copy data from host memory to a buffer's device memory. If the buffer's
/// memory is host coherent the data is written right away, otherwise it is
/// copied to staging memory and the copy to the buffer is recorded with other
/// uploads, see gfxFlushUploads(). Either way pData can be reused when the
/// function returns.
/// </summary>
/// <param name="pBuffer">Buffer to use</param>
/// <param name="pData">Pointer to data to copy</param>
/// <param name="size">Size of data to copy in bytes</param>
/// <param name="offset">Offset into buffer's device memory to put the data</param>
void gfxCopyBufferFromHost(const GfxBuffer* pBuffer, const void* pData, VkDeviceSize size, VkDeviceSize offset);

/// <summary>
/// Create a buffer for data that changes every frame. It holds one copy per
/// frame in flight in host coherent memory, so the swapchain has to be created
/// first. Without a swapchain it holds a single copy.
/// </summary>
/// <param name="size">Size of one copy in bytes</param>
/// <param name="usage">How the buffer will be used</param>
/// <param name="pBuffer">Where the created buffer will be stored</param>
void gfxCreatePerFrameBuffer(VkDeviceSize size, VkBufferUsageFlags usage, GfxBuffer* pBuffer);

/// <summary>
/// Get the copy of a per-frame buffer that belongs to the frame being
/// recorded. Once gfxWaitForFence() or gfxAcquireNextImage() has returned, the
/// device no longer reads it, so it can be written freely.
/// </summary>
/// <param name="pBuffer">Per-frame buffer to use</param>
/// <returns>Host pointer to this frame's copy</returns>
void* gfxGetPerFrameData(const GfxBuffer* pBuffer);

/// <summary>
/// Get the offset of this frame's copy of a per-frame buffer, for binding it
/// as for instance a vertex buffer. Ordinary buffers have offset 0.
/// </summary>
/// <param name="pBuffer">Buffer to use</param>
/// <returns>Offset in bytes</returns>
VkDeviceSize gfxGetPerFrameOffset(const GfxBuffer* pBuffer);

/// <summary>
/// Get the device address of a buffer. The buffer must have been created with
/// VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT.
/// </summary>
/// <param name="pBuffer">Buffer to use</param>
/// <returns>Device address of the start of the buffer</returns>
VkDeviceAddress gfxGetBufferDeviceAddress(const GfxBuffer* pBuffer);

/// <summary>
/// Create a new image.
/// </summary>
/// <param name="extent">Size of the image. Height and depth must be 1 for 1D, depth must be 1 for 1D and 2D</param>
/// <param name="arrayLayers">Number of array layers, 6 for a cube compatible image</param>
/// <param name="mipLevels">Mipmap levels to use</param>
/// <param name="samples">Sample count</param>
/// <param name="format">Format to use</param>
/// <param name="tiling">Tiling to use</param>
/// <param name="usage">How the image will be used</param>
/// <param name="flags">Image create flags, for instance VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT</param>
/// <param name="imageType">Whether the image is 1D, 2D or 3D</param>
/// <param name="properties">Properties of the memory to allocate</param>
/// <param name="pImage">Where the created image will be stored</param>
void gfxCreateImage(VkExtent3D extent, uint32_t arrayLayers, uint32_t mipLevels, VkSampleCountFlagBits samples,
                    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                    VkImageType imageType, VkMemoryPropertyFlags properties, GfxImage* pImage);

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
/// <param name="viewType">Type of view to create, must be compatible with how the image was created</param>
void gfxCreateImageView(GfxImage* pImage, VkImageAspectFlags aspectFlags, VkImageViewType viewType);

/// <summary>
/// Copy the first mip level and array layer of an image to host memory. The
/// copy is submitted and waited for, after all work submitted before it,
/// including pending uploads.
/// </summary>
/// <param name="pImage">Image to copy</param>
/// <param name="layout">Layout the image is in, which it is left in</param>
/// <param name="pData">Where to copy to, width * height * depth * gfxFormatSize(format) bytes</param>
void gfxCopyImageToHost(const GfxImage* pImage, VkImageLayout layout, void* pData);

#ifdef GFX_USE_STB_IMAGE_WRITE
/// <summary>
/// Save an image to a PNG file, for instance to take a screenshot of what is
/// presented. The image must have an 8 bit RGBA or BGRA format.
/// </summary>
/// <param name="pImage">Image to save</param>
/// <param name="layout">Layout the image is in, which it is left in</param>
/// <param name="pPath">Path of the PNG file to write</param>
/// <returns>True if the file was written</returns>
bool gfxSaveImagePNG(const GfxImage* pImage, VkImageLayout layout, const char* pPath);
#endif

/// <summary>
/// Create a new texture. For a cube map, pData holds the six faces in the order
/// +X, -X, +Y, -Y, +Z, -Z.
/// </summary>
/// <param name="type">Type of texture to create</param>
/// <param name="format">Format to use</param>
/// <param name="pData">Pointer to data with texture pixel values, can be NULL</param>
/// <param name="extent">Size of the texture. Height must be 1 for 1D, depth must be 1 for anything but 3D</param>
/// <param name="generateMipmaps">Whether to generate mipmaps or not</param>
/// <param name="pTexture">Where the created texture will be stored</param>
void gfxCreateTexture(enum GfxTextureType type, VkFormat format, const void* pData, VkExtent3D extent,
                      bool generateMipmaps, GfxTexture* pTexture);

#ifdef GFX_USE_STB_IMAGE
/// <summary>
/// Create a new texture from file. Only GFX_TEXTURE_2D is supported. The file
/// is loaded with four channels, as 8 bit for VK_FORMAT_R8G8B8A8_UNORM and
/// VK_FORMAT_R8G8B8A8_SRGB, 16 bit for VK_FORMAT_R16G16B16A16_UNORM and as
/// floats for VK_FORMAT_R32G32B32A32_SFLOAT, which also loads HDR files.
/// </summary>
/// <param name="type">Type of texture to create, must be GFX_TEXTURE_2D</param>
/// <param name="format">Format to use, one of the above</param>
/// <param name="pPath">Path to texture file</param>
/// <param name="generateMipmaps">Whether to generate mipmaps or not</param>
/// <param name="pTexture">Where the created texture will be stored</param>
void gfxCreateTextureFromFile(enum GfxTextureType type, VkFormat format, const char* pPath, bool generateMipmaps,
                              GfxTexture* pTexture);
#endif

/// <summary>
/// Create a sampler, for sampling images that are not textures, such as the
/// images of a render graph. A comparing sampler is meant for shadow maps
/// sampled with sampler2DShadow: it returns 1 where the reference depth is
/// nearer than, or as near as, the depth stored, following GFX_REVERSE_Z, and
/// its border is at the far depth, so lookups outside a clamped shadow map are
/// lit. Release it with gfxDestroySampler().
/// </summary>
/// <param name="filter">Magnification and minification filter, linear filtering of a comparing sampler gives
/// smoothed shadow edges</param>
/// <param name="addressMode">Address mode in all directions</param>
/// <param name="compare">Whether the sampler compares against a reference depth</param>
/// <param name="pSampler">Where the created sampler will be stored</param>
void gfxCreateSampler(VkFilter filter, VkSamplerAddressMode addressMode, bool compare, VkSampler* pSampler);

/// <summary>
/// Release a sampler created with gfxCreateSampler().
/// </summary>
/// <param name="sampler">Sampler to destroy</param>
void gfxDestroySampler(VkSampler sampler);

/// <summary>
/// Release resources for a texture.
/// </summary>
/// <param name="pTexture">Texture to destroy</param>
void gfxDestroyTexture(GfxTexture* pTexture);

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
/// Create a new attachment set.
/// </summary>
/// <param name="colorAttachmentCount">Number of color attachments</param>
/// <param name="pColorAttachments">List of images that should be used as color attachments</param>
/// <param name="pDepthAttachment">Depth attachment to use, can be NULL</param>
/// <param name="pResolveAttachment">Resolve attachment to use, can be NULL</param>
/// <param name="pAttachmentSet">Where the created attachment set will be stored</param>
void gfxCreateAttachment(uint32_t colorAttachmentCount, GfxImage* pColorAttachments, GfxImage* pDepthAttachment,
                         GfxImage* pResolveAttachment, GfxAttachment* pAttachmentSet);

/// <summary>
/// Release resource for an attachment set.
/// </summary>
/// <param name="pAttachmentSet">Attachment set to destroy</param>
void gfxDestroyAttachment(GfxAttachment* pAttachmentSet);

/// <summary>
/// Recreate a pass if attachments changed. Same as calling gfxDestroyAttachment()
/// followed by gfxCreateAttachment().
/// </summary>
/// <param name="pAttachmentSet">Attachment set to recreate</param>
/// <param name="colorAttachmentCount">Number of color attachments</param>
/// <param name="pColorAttachments">List of images that should be used as color attachments</param>
/// <param name="pDepthAttachment">Depth attachment to use, can be NULL</param>
/// <param name="pResolveAttachment">Resolve attachment to use, can be NULL</param>
void gfxRecreateAttachment(GfxAttachment* pAttachmentSet, uint32_t colorAttachmentCount, GfxImage* pColorAttachments,
                           GfxImage* pDepthAttachment, GfxImage* pResolveAttachment);

/// <summary>
/// Begin dynamic rendering using an attachment set. A depth attachment is
/// cleared to GFX_DEPTH_CLEAR.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pAttachmentSet">Attachment set to use</param>
/// <param name="clearValue">Clear value for attachments</param>
/// <param name="loadOp">Load operation for attachments</param>
void gfxCmdBeginRendering(VkCommandBuffer cmd, const GfxAttachment* pAttachmentSet, VkClearValue clearValue,
                          VkAttachmentLoadOp loadOp);

/// <summary>
/// End dynamic rendering using an attachment set.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pAttachmentSet">Attachment set to use</param>
void gfxCmdEndRendering(VkCommandBuffer cmd, const GfxAttachment* pAttachmentSet);

/// <summary>
/// Create a new shader from one or more stages. GLSL is compiled to SPIR-V,
/// and descriptor set layouts, the pipeline layout and the push constant range
/// are reflected from the SPIR-V. Graphics stages are linked together, a
/// compute stage must be on its own and ray tracing stages are only compiled
/// and reflected, to be built into a ray tracing pipeline.
/// </summary>
/// <param name="stageCount">Number of stages</param>
/// <param name="pStages">List of stage descriptions</param>
/// <param name="pShader">Where the created shader will be stored</param>
void gfxCreateShader(uint32_t stageCount, const GfxShaderStageDesc* pStages, GfxShader* pShader);

/// <summary>
/// Release resources for a shader.
/// </summary>
/// <param name="pShader">Shader to destroy</param>
void gfxDestroyShader(GfxShader* pShader);

/// <summary>
/// Recompile a shader from its source files and rebuild it. Resources attached
/// to the shader are kept and attached again by name. If anything fails, the
/// errors are reported and the shader is left as it was. Must not be called
/// while a command buffer that uses the shader is being recorded.
/// </summary>
/// <param name="pShader">Shader to reload</param>
/// <returns>True if the shader was reloaded</returns>
bool gfxReloadShader(GfxShader* pShader);

/// <summary>
/// Look up a reflected binding by name.
/// </summary>
/// <param name="pShader">Shader to use</param>
/// <param name="pName">Variable name, or block type name for blocks without an instance name</param>
/// <returns>The binding, or NULL if the shader has no resource with that name</returns>
const GfxShaderBinding* gfxGetShaderBinding(const GfxShader* pShader, const char* pName);

/// <summary>
/// Attach a whole buffer to a named uniform or storage buffer in the shader.
/// </summary>
/// <param name="pShader">Shader to use</param>
/// <param name="pName">Variable name, or block type name for blocks without an instance name</param>
/// <param name="pBuffer">Buffer to attach, must stay at the same address while attached</param>
void gfxSetShaderBuffer(GfxShader* pShader, const char* pName, const GfxBuffer* pBuffer);

/// <summary>
/// Attach a range of a buffer to a named uniform or storage buffer in the
/// shader.
/// </summary>
/// <param name="pShader">Shader to use</param>
/// <param name="pName">Variable name, or block type name for blocks without an instance name</param>
/// <param name="pBuffer">Buffer to attach, must stay at the same address while attached</param>
/// <param name="offset">Offset into the buffer</param>
/// <param name="range">Range in bytes, can be VK_WHOLE_SIZE</param>
void gfxSetShaderBufferRange(GfxShader* pShader, const char* pName, const GfxBuffer* pBuffer, VkDeviceSize offset,
                             VkDeviceSize range);

/// <summary>
/// Attach a texture to a named combined image sampler, sampled image or
/// sampler in the shader.
/// </summary>
/// <param name="pShader">Shader to use</param>
/// <param name="pName">Variable name</param>
/// <param name="pTexture">Texture to attach, must stay at the same address while attached</param>
void gfxSetShaderTexture(GfxShader* pShader, const char* pName, const GfxTexture* pTexture);

/// <summary>
/// Attach an array of textures to a named array of combined image samplers,
/// sampled images or samplers in the shader.
/// </summary>
/// <param name="pShader">Shader to use</param>
/// <param name="pName">Variable name</param>
/// <param name="textureCount">Number of textures, must match the array size in the shader</param>
/// <param name="pTextures">Textures to attach, must stay at the same address while attached</param>
void gfxSetShaderTextureArray(GfxShader* pShader, const char* pName, uint32_t textureCount,
                              const GfxTexture* pTextures);

/// <summary>
/// Attach an image to a named storage image, sampled image or input
/// attachment in the shader. The image must have an image view.
/// </summary>
/// <param name="pShader">Shader to use</param>
/// <param name="pName">Variable name</param>
/// <param name="pImage">Image to attach, must stay at the same address while attached</param>
/// <param name="layout">Layout the image is in when the shader accesses it. VK_IMAGE_LAYOUT_UNDEFINED picks
/// VK_IMAGE_LAYOUT_GENERAL for storage images and VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL otherwise.</param>
void gfxSetShaderImage(GfxShader* pShader, const char* pName, const GfxImage* pImage, VkImageLayout layout);

/// <summary>
/// Attach an image together with a sampler to a named combined image sampler
/// in the shader, for instance a render graph's shadow map to a
/// sampler2DShadow. The image must have an image view.
/// </summary>
/// <param name="pShader">Shader to use</param>
/// <param name="pName">Variable name</param>
/// <param name="pImage">Image to attach, must stay at the same address while attached</param>
/// <param name="sampler">Sampler to sample it with, for instance from gfxCreateSampler()</param>
/// <param name="layout">Layout the image is in when the shader samples it. VK_IMAGE_LAYOUT_UNDEFINED picks
/// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, which is what a render graph reads sampled images in.</param>
void gfxSetShaderImageSampler(GfxShader* pShader, const char* pName, const GfxImage* pImage, VkSampler sampler,
                              VkImageLayout layout);

/// <summary>
/// Attach a top level acceleration structure to a named acceleration structure
/// in the shader.
/// </summary>
/// <param name="pShader">Shader to use</param>
/// <param name="pName">Variable name</param>
/// <param name="pAccelerationStructure">Acceleration structure to attach, must stay at the same address while
/// attached</param>
void gfxSetShaderAccelerationStructure(GfxShader* pShader, const char* pName,
                                       const GfxAccelerationStructure* pAccelerationStructure);

/// <summary>
/// Bind a shader. For graphics shaders, the graphics stages that the shader
/// does not have are bound to null. Ray tracing shaders cannot be bound like
/// this, they are used through a ray tracing pipeline.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pShader">Shader to bind</param>
void gfxCmdBindShader(VkCommandBuffer cmd, const GfxShader* pShader);

/// <summary>
/// Bind the descriptor sets of a shader with the resources currently attached.
/// A set is only rewritten if what is attached to it changed since it was last
/// written. Resources that have nothing attached are reported once.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pShader">Shader whose resources to bind</param>
void gfxCmdBindResources(VkCommandBuffer cmd, GfxShader* pShader);

/// <summary>
/// Update push constants of a shader.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pShader">Shader to use</param>
/// <param name="offset">Offset in bytes into the push constant block</param>
/// <param name="size">Size of data in bytes</param>
/// <param name="pData">Data to push</param>
void gfxCmdPushConstants(VkCommandBuffer cmd, const GfxShader* pShader, uint32_t offset, uint32_t size,
                         const void* pData);

/// <summary>
/// Dispatch a compute shader over a number of work items. The counts are
/// rounded up to whole workgroups of the shader's reflected local size, so the
/// shader has to ignore invocations beyond the work items.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pShader">Bound compute shader</param>
/// <param name="x">Number of work items in x</param>
/// <param name="y">Number of work items in y</param>
/// <param name="z">Number of work items in z</param>
void gfxCmdDispatch(VkCommandBuffer cmd, const GfxShader* pShader, uint32_t x, uint32_t y, uint32_t z);

/// <summary>
/// Create a bottom level acceleration structure from triangle geometry. The
/// build is recorded with other uploads, after any upload of the geometry
/// before it, see gfxFlushUploads(). Builds of several structures can overlap on
/// the device.
/// </summary>
/// <param name="geometryCount">Number of geometries</param>
/// <param name="pGeometries">List of geometries</param>
/// <param name="flags">Build flags, for instance VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR</param>
/// <param name="pBLAS">Where the created acceleration structure will be stored</param>
void gfxCreateBLAS(uint32_t geometryCount, const GfxGeometryDesc* pGeometries,
                   VkBuildAccelerationStructureFlagsKHR flags, GfxAccelerationStructure* pBLAS);

/// <summary>
/// Create a top level acceleration structure from instances of bottom level
/// acceleration structures. The build is recorded with other uploads, after the
/// builds of bottom level structures before it, see gfxFlushUploads(). Use
/// gfxAccelerationStructureInstance() to fill in instances.
/// </summary>
/// <param name="instanceCount">Number of instances, also the most that later updates can have</param>
/// <param name="pInstances">List of instances</param>
/// <param name="flags">Build flags. With VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR, updates that keep
/// the instance count refit the structure instead of rebuilding it.</param>
/// <param name="pTLAS">Where the created acceleration structure will be stored</param>
void gfxCreateTLAS(uint32_t instanceCount, const VkAccelerationStructureInstanceKHR* pInstances,
                   VkBuildAccelerationStructureFlagsKHR flags, GfxAccelerationStructure* pTLAS);

/// <summary>
/// Record a change of the instances of a top level acceleration structure.
/// The instances are copied into the command buffer, and the update is
/// synchronized with earlier and later use of the acceleration structure.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pTLAS">Top level acceleration structure to update</param>
/// <param name="instanceCount">Number of instances, at most what the TLAS was created with</param>
/// <param name="pInstances">List of instances</param>
void gfxCmdUpdateTLAS(VkCommandBuffer cmd, GfxAccelerationStructure* pTLAS, uint32_t instanceCount,
                      const VkAccelerationStructureInstanceKHR* pInstances);

/// <summary>
/// Release resources for an acceleration structure.
/// </summary>
/// <param name="pAccelerationStructure">Acceleration structure to destroy</param>
void gfxDestroyAccelerationStructure(GfxAccelerationStructure* pAccelerationStructure);

/// <summary>
/// Create a ray tracing pipeline and its shader binding table from a shader
/// with ray tracing stages. The shader must have exactly one ray generation
/// stage.
/// </summary>
/// <param name="pShader">Shader to use, must stay at the same address while the pipeline exists</param>
/// <param name="maxRecursionDepth">How deep traceRayEXT() calls can be nested, 1 if only the ray generation
/// stage traces rays</param>
/// <param name="pPipeline">Where the created pipeline will be stored</param>
void gfxCreateRayTracingPipeline(GfxShader* pShader, uint32_t maxRecursionDepth, GfxRayTracingPipeline* pPipeline);

/// <summary>
/// Release resources for a ray tracing pipeline. The shader is not destroyed.
/// </summary>
/// <param name="pPipeline">Pipeline to destroy</param>
void gfxDestroyRayTracingPipeline(GfxRayTracingPipeline* pPipeline);

/// <summary>
/// Bind a ray tracing pipeline. If its shader has been reloaded since the
/// pipeline was built, the pipeline is rebuilt first.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pPipeline">Pipeline to bind</param>
void gfxCmdBindRayTracingPipeline(VkCommandBuffer cmd, GfxRayTracingPipeline* pPipeline);

/// <summary>
/// Trace rays with the bound ray tracing pipeline.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pPipeline">Pipeline whose shader binding table to use</param>
/// <param name="width">Number of rays in x</param>
/// <param name="height">Number of rays in y</param>
/// <param name="depth">Number of rays in z</param>
void gfxCmdTraceRays(VkCommandBuffer cmd, const GfxRayTracingPipeline* pPipeline, uint32_t width, uint32_t height,
                     uint32_t depth);

/// <summary>
/// Set default states for rendering using shader objects. Viewport, scissor and
/// sample count are taken from the attachment set's first color attachment, or
/// from its depth attachment if it has no color attachments.
/// Depth testing is enabled with GFX_DEPTH_COMPARE_OP, which follows the depth
/// convention chosen with GFX_REVERSE_Z. Blending is disabled, but the blend
/// equation is set to the usual alpha blending, so enabling blending with
/// gfxDevice.fn.vkCmdSetColorBlendEnableEXT() is enough to get it.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pAttachmentSet">Attachment set being rendered to</param>
/// <param name="vertexBindingDescriptionCount">Number of vertex binding descriptions</param>
/// <param name="vertexBindingDescriptions">List of vertex binding descriptions</param>
/// <param name="vertexAttributeDescriptionCount">Number of vertex attribute descriptions</param>
/// <param name="vertexAttributeDescriptions">List of vertex attribute descriptions</param>
void gfxCmdSetDefaultStates(VkCommandBuffer cmd, const GfxAttachment* pAttachmentSet,
                            uint32_t vertexBindingDescriptionCount,
                            const VkVertexInputBindingDescription2EXT* vertexBindingDescriptions,
                            uint32_t vertexAttributeDescriptionCount,
                            const VkVertexInputAttributeDescription2EXT* vertexAttributeDescriptions);

/// <summary>
/// Set whether the swapchain should present with vertical synchronization. If a
/// swapchain already exists it is recreated to pick up the new present mode.
/// Off by default.
/// </summary>
/// <param name="vsync">Whether to wait for vertical blank when presenting</param>
void gfxSetVsync(bool vsync);

/// <summary>
/// Create an empty render graph.
/// </summary>
/// <param name="pGraph">Where the created render graph will be stored</param>
void gfxCreateRenderGraph(GfxRenderGraph* pGraph);

/// <summary>
/// Release resources for a render graph, including the images it owns.
/// </summary>
/// <param name="pGraph">Render graph to destroy</param>
void gfxDestroyRenderGraph(GfxRenderGraph* pGraph);

/// <summary>
/// Add an image to a render graph. The image is created right away, so that it
/// can be attached to shaders, and stays at the same address for as long as the
/// graph exists, also when it is recreated for a new swapchain size.
/// </summary>
/// <param name="pGraph">Render graph to use</param>
/// <param name="pDesc">Description of the image</param>
/// <returns>The image</returns>
GfxImage* gfxAddGraphImage(GfxRenderGraph* pGraph, const GfxGraphImageDesc* pDesc);

/// <summary>
/// Add a buffer to a render graph, so that passes reading and writing it are
/// ordered and synchronized. The buffer stays owned by the application.
/// </summary>
/// <param name="pGraph">Render graph to use</param>
/// <param name="pName">Name passes refer to the buffer by</param>
/// <param name="pBuffer">Buffer to add, must stay at the same address while the graph exists</param>
void gfxAddGraphBuffer(GfxRenderGraph* pGraph, const char* pName, const GfxBuffer* pBuffer);

/// <summary>
/// Add a pass to a render graph. The order passes are added in does not
/// matter, they run after the passes that write what they read.
/// </summary>
/// <param name="pGraph">Render graph to use</param>
/// <param name="pDesc">Description of the pass</param>
void gfxAddGraphPass(GfxRenderGraph* pGraph, const GfxPassDesc* pDesc);

/// <summary>
/// Set the image that leaves the graph, which is left in
/// VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ready for gfxPresent().
/// </summary>
/// <param name="pGraph">Render graph to use</param>
/// <param name="pName">Name of the image</param>
void gfxSetGraphOutput(GfxRenderGraph* pGraph, const char* pName);

/// <summary>
/// Get an image of a render graph by name.
/// </summary>
/// <param name="pGraph">Render graph to use</param>
/// <param name="pName">Name of the image</param>
/// <returns>The image, or NULL if the graph has no image with that name</returns>
GfxImage* gfxGetGraphImage(const GfxRenderGraph* pGraph, const char* pName);

/// <summary>
/// Record every pass of a render graph, in the order the data flows between
/// them, with the transitions and barriers they need. The graph is compiled
/// the first time, and images following the swapchain size are recreated if it
/// changed.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pGraph">Render graph to execute</param>
void gfxCmdExecuteRenderGraph(VkCommandBuffer cmd, GfxRenderGraph* pGraph);

/// <summary>
/// Give a Vulkan object a name, which validation messages, debuggers such as
/// RenderDoc and profilers such as Nsight show. Does nothing if VK_EXT_debug_utils
/// is not available. Objects created by GFX from files and the images of
/// render graphs are named already.
/// </summary>
/// <param name="type">Type of the object</param>
/// <param name="handle">Handle of the object, cast to uint64_t</param>
/// <param name="pName">Name to give it</param>
void gfxSetDebugName(VkObjectType type, uint64_t handle, const char* pName);

/// <summary>
/// Name a buffer, and its memory if it has memory of its own, see
/// gfxSetDebugName().
/// </summary>
/// <param name="pBuffer">Buffer to name</param>
/// <param name="pName">Name to give it</param>
void gfxSetBufferName(const GfxBuffer* pBuffer, const char* pName);

/// <summary>
/// Name an image, its image view, and its memory if it has memory of its own,
/// see gfxSetDebugName().
/// </summary>
/// <param name="pImage">Image to name</param>
/// <param name="pName">Name to give it</param>
void gfxSetImageName(const GfxImage* pImage, const char* pName);

/// <summary>
/// Begin a labeled region of a command buffer, which debuggers and profilers
/// show the commands in. Regions can be nested, and are ended with
/// gfxCmdEndLabel(). Render graphs label each pass with its name.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="pName">Name of the region</param>
void gfxCmdBeginLabel(VkCommandBuffer cmd, const char* pName);

/// <summary>
/// End the region begun by the last gfxCmdBeginLabel().
/// </summary>
/// <param name="cmd">Command buffer to use</param>
void gfxCmdEndLabel(VkCommandBuffer cmd);

/// <summary>
/// Begin measuring the GPU time of the commands that follow, in the command
/// buffer of the frame returned by gfxAcquireNextImage(). Timers can be nested
/// and are ended with gfxCmdEndTimer(). Does nothing if the queue does not
/// support timestamps.
/// </summary>
/// <param name="cmd">Command buffer of the frame</param>
/// <param name="pName">Name of the timer, at most GFX_MAX_TIMER_NAME_LENGTH - 1 characters are kept</param>
void gfxCmdBeginTimer(VkCommandBuffer cmd, const char* pName);

/// <summary>
/// End the timer begun by the last gfxCmdBeginTimer().
/// </summary>
/// <param name="cmd">Command buffer of the frame</param>
void gfxCmdEndTimer(VkCommandBuffer cmd);

/// <summary>
/// Get the GPU timers of the latest finished frame, which is the frame that
/// was recorded a number of frames in flight ago. Timers are in the order they
/// were begun, so a timer is followed by the timers nested in it.
/// </summary>
/// <param name="ppResults">Where a pointer to the results will be stored, valid until the next
/// gfxAcquireNextImage()</param> <returns>Number of results, 0 before the first frame has finished</returns>
uint32_t gfxGetTimerResults(const GfxTimerResult** ppResults);

// Size of the staging memory of an upload batch. A batch is submitted when its
// staging memory is full, and larger uploads get staging memory of their own.
#ifndef GFX_UPLOAD_STAGING_SIZE
#define GFX_UPLOAD_STAGING_SIZE (32ull * 1024 * 1024)
#endif

// Number of upload batches that can be in flight before recording another
// waits for the oldest to finish
#ifndef GFX_UPLOAD_BATCHES
#define GFX_UPLOAD_BATCHES 3
#endif

/// <summary>
/// Submit the uploads recorded so far, and optionally wait for them.
///
/// Uploads that go through the device, copies to buffers that are not host
/// coherent, texture data with its mipmaps, and acceleration structure builds,
/// are recorded into a batch instead of being waited for one by one. The data
/// is copied to staging memory right away, so the caller's memory can be
/// reused as soon as the call returns. A batch is submitted on the same queue
/// as the frames, with barriers that order it after everything submitted
/// before it and before everything submitted after it, so frames and command
/// buffers see the uploads without waiting for them.
///
/// Batches are submitted by gfxPresent() before the frame, by gfxCmdEnd(), by
/// the destroy functions, and when the staging memory of a batch is full. Call
/// this before submitting command buffers of your own that use the uploads, or
/// with wait to have them finished, for instance before measuring load times.
/// </summary>
/// <param name="wait">Whether to wait for every submitted upload to finish</param>
void gfxFlushUploads(bool wait);

// Inlined helper functions //

/// <summary>
/// Call malloc and check for valid pointer.
/// </summary>
/// <param name="sz">Size in bytes to allocate</param>
/// <returns>Pointer to allocated memory</returns>
static inline void* gfxCheckedMalloc(size_t sz)
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
static inline VkDeviceSize gfxAlignTo(VkDeviceSize value, VkDeviceSize alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

/// <summary>
/// Create and begin a new one-time-use command buffer.
/// </summary>
/// <returns>A new command buffer</returns>
static inline VkCommandBuffer gfxCmdBegin()
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
/// Submit a one-time-use command buffer, after any pending uploads, wait for it
/// and destroy it.
/// </summary>
/// <param name="cmd">Command buffer to submit and destroy</param>
static inline void gfxCmdEnd(VkCommandBuffer cmd)
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    VK_CHECK(vkEndCommandBuffer(cmd));

    gfxFlushUploads(false);

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
static inline uint32_t gfxFindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
/// <param name="features">Format feature flags</param>
/// <returns>A supported format</returns>
static inline VkFormat gfxFindSupportedFormat(VkFormat* pCandidates, uint32_t candidateCount, VkImageTiling tiling,
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
static inline VkFormat gfxFindDepthFormat()
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
/// Get the size of one texel of an uncompressed format.
/// </summary>
/// <param name="format">Format to use</param>
/// <returns>Size in bytes, or 0 for compressed, multi-planar and combined depth stencil formats</returns>
static inline uint32_t gfxFormatSize(VkFormat format)
{
    // Core formats of the same texel size are numbered consecutively
    struct GfxFormatRange {
        VkFormat first;
        VkFormat last;
        uint32_t size;
    } ranges[] = {
        {VK_FORMAT_R4G4_UNORM_PACK8, VK_FORMAT_R4G4_UNORM_PACK8, 1},
        {VK_FORMAT_R4G4B4A4_UNORM_PACK16, VK_FORMAT_A1R5G5B5_UNORM_PACK16, 2},
        {VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SRGB, 1},
        {VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SRGB, 2},
        {VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_B8G8R8_SRGB, 3},
        {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_A2B10G10R10_SINT_PACK32, 4},
        {VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SFLOAT, 2},
        {VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SFLOAT, 4},
        {VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SFLOAT, 6},
        {VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SFLOAT, 8},
        {VK_FORMAT_R32_UINT, VK_FORMAT_R32_SFLOAT, 4},
        {VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SFLOAT, 8},
        {VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_SFLOAT, 12},
        {VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SFLOAT, 16},
        {VK_FORMAT_R64_UINT, VK_FORMAT_R64_SFLOAT, 8},
        {VK_FORMAT_R64G64_UINT, VK_FORMAT_R64G64_SFLOAT, 16},
        {VK_FORMAT_R64G64B64_UINT, VK_FORMAT_R64G64B64_SFLOAT, 24},
        {VK_FORMAT_R64G64B64A64_UINT, VK_FORMAT_R64G64B64A64_SFLOAT, 32},
        {VK_FORMAT_B10G11R11_UFLOAT_PACK32, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, 4},
        {VK_FORMAT_D16_UNORM, VK_FORMAT_D16_UNORM, 2},
        {VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT, 4},
        {VK_FORMAT_S8_UINT, VK_FORMAT_S8_UINT, 1},
    };

    for (uint32_t i = 0; i < GFX_ARRAY_LEN(ranges); i++) {
        if (format >= ranges[i].first && format <= ranges[i].last) {
            return ranges[i].size;
        }
    }
    return 0;
}

/// <summary>
/// Create a global memory barrier.
/// </summary>
/// <param name="cmd">Command buffer to use</param>
/// <param name="srcStage">Stage to wait for</param>
/// <param name="srcAccess">Type of access to wait for</param>
/// <param name="dstStage">Stage before which the barrier has to take place</param>
/// <param name="dstAccess">Type of access before which the barrier has to take place</param>
static inline void gfxMemoryBarrier(VkCommandBuffer cmd, VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                                    VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess)
{
    VkMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = srcStage,
        .srcAccessMask = srcAccess,
        .dstStageMask = dstStage,
        .dstAccessMask = dstAccess,
    };

    VkDependencyInfo dependencyInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .memoryBarrierCount = 1,
        .pMemoryBarriers = &barrier,
    };

    vkCmdPipelineBarrier2(cmd, &dependencyInfo);
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
/// <param name="pSubresourceRange">Subresource range to use, can be NULL in which case all mip levels and array
/// layers of the color aspect are used.</param>
static inline void gfxImageBarrier(VkCommandBuffer cmd, VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                                   VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess, VkImageLayout oldLayout,
                                   VkImageLayout newLayout, VkImage image, VkImageSubresourceRange* pSubresourceRange)
{
    VkImageSubresourceRange defaultSubresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
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
static inline void gfxTransitionForColorAttachment(VkCommandBuffer cmd, GfxImage* pImage)
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
static inline void gfxTransitionForBlitting(VkCommandBuffer cmd, GfxImage* pImage)
{
    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pImage->image,
                    NULL);
}

/// <summary>
/// Convert a column major 4x4 matrix, as used by GLSL and most math libraries,
/// to the row major 3x4 matrix of an acceleration structure instance.
/// </summary>
/// <param name="pColumnMajor">16 floats in column major order</param>
/// <returns>The transform of an instance</returns>
static inline VkTransformMatrixKHR gfxTransformMatrix(const float* pColumnMajor)
{
    VkTransformMatrixKHR transform;
    for (uint32_t row = 0; row < 3; row++) {
        for (uint32_t column = 0; column < 4; column++) {
            transform.matrix[row][column] = pColumnMajor[column * 4 + row];
        }
    }
    return transform;
}

/// <summary>
/// Make an instance of a bottom level acceleration structure for a top level
/// acceleration structure. The instance is visible to all rays, uses the first
/// hit group and has culling disabled; change the returned instance for
/// anything else.
/// </summary>
/// <param name="pBLAS">Bottom level acceleration structure to instance</param>
/// <param name="pTransform">Column major 4x4 transform, as 16 floats</param>
/// <param name="customIndex">Value of gl_InstanceCustomIndexEXT, 24 bits</param>
/// <returns>The instance</returns>
static inline VkAccelerationStructureInstanceKHR
gfxAccelerationStructureInstance(const GfxAccelerationStructure* pBLAS, const float* pTransform, uint32_t customIndex)
{
    VkAccelerationStructureInstanceKHR instance = {
        .transform = gfxTransformMatrix(pTransform),
        .instanceCustomIndex = customIndex & 0xffffff,
        .mask = 0xff,
        .instanceShaderBindingTableRecordOffset = 0,
        .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
        .accelerationStructureReference = pBLAS->address,
    };
    return instance;
}

// Define GFX_IMPLEMENTATION in exactly one compilation unit before
// including gfx.h
#ifdef GFX_IMPLEMENTATION


// Monolithic global variables //

// Definitions for the extern declarations above. These must not be static, so
// that every translation unit including this header shares the same objects.
GfxInstance gfxInstance;
GfxDevice gfxDevice;
GfxSwapchain gfxSwapchain;

// GPU timers of one frame in flight. Timer i writes queries 2i and 2i + 1.
typedef struct GfxTimerFrame {
    VkQueryPool queryPool;
    uint32_t count;
    GfxTimerResult timers[GFX_MAX_TIMERS]; // Names and depths, timed when read back
} GfxTimerFrame;

static struct GfxTimers {
    GfxTimerFrame* pFrames;         // One per frame in flight, NULL without timestamps
    uint32_t stack[GFX_MAX_TIMERS]; // Timers begun and not ended, UINT32_MAX for ones that did not fit
    uint32_t depth;
    bool reported; // Whether running out of timers has been reported
    uint32_t resultCount;
    GfxTimerResult results[GFX_MAX_TIMERS];
} gfxTimers;

// Uploads recorded into one command buffer and submitted with a fence. The
// staging memory is kept and reused, and the retired buffers are released,
// once the fence has signaled.
typedef struct GfxUploadBatch {
    VkCommandBuffer cmd;
    VkFence fence;
    bool submitted;    // Submitted and not yet seen to have finished
    GfxBuffer staging; // Created the first time it is needed
    VkDeviceSize used; // Bytes of the staging memory taken
    uint32_t retiredCount;
    uint32_t retiredCapacity;
    GfxBuffer* pRetired; // Released when the batch has finished
    uint32_t writtenCount;
    uint32_t writtenCapacity;
    VkBuffer* pWritten; // Buffers copied to since the last barrier between copies
    bool transferred;   // Whether copies were recorded since the last barrier before builds
    bool built;         // Whether builds were recorded since the last barrier before copies
} GfxUploadBatch;

static struct GfxUploads {
    GfxUploadBatch batches[GFX_UPLOAD_BATCHES];
    uint32_t current; // Batch being recorded, or the one recorded next
    bool recording;
} gfxUploads;

typedef struct GfxMemoryRange {
    VkDeviceSize offset;
    VkDeviceSize size;
} GfxMemoryRange;

// Device memory that buffers and images are placed in, with the ranges of it
// that are free
typedef struct GfxMemoryBlock {
    VkDeviceMemory memory;
    VkDeviceSize size;
    uint32_t memoryTypeIndex;
    bool optimal;  // Holds images with optimal tiling, and nothing else
    void* pMapped; // The whole block, if the memory is host visible
    uint32_t allocationCount;
    uint32_t freeCount;
    uint32_t freeCapacity;
    GfxMemoryRange* pFree; // Sorted by offset, never adjacent
} GfxMemoryBlock;

static struct GfxMemory {
    uint32_t blockCount;
    uint32_t blockCapacity;
    GfxMemoryBlock* pBlocks;
    uint32_t dedicatedCount; // Allocations with memory of their own
} gfxMemory;

typedef struct GfxMemoryAllocation {
    VkDeviceMemory memory;
    VkDeviceSize offset;
    VkDeviceSize size;
    void* pMapped; // At the offset, if the memory is host visible
} GfxMemoryAllocation;

static void destroyUploadBatches();
static void destroyMemoryBlocks();


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

    VK_CHECK(createDebugUtilsMessengerEXT(gfxInstance.instance, &ci, NULL, &gfxInstance.debugMessenger));
}
#endif

static bool extensionInList(uint32_t extensionCount, const char** ppExtensions, const char* pName)
{
    for (uint32_t i = 0; i < extensionCount; i++) {
        if (!strcmp(pName, ppExtensions[i])) {
            return true;
        }
    }
    return false;
}

static bool instanceExtensionAvailable(const char* pName)
{
    uint32_t n;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &n, NULL));
    VkExtensionProperties* pAvailable = GFX_MALLOC(n * sizeof *pAvailable);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &n, pAvailable));

    bool found = false;
    for (uint32_t i = 0; i < n && !found; i++) {
        found = !strcmp(pName, pAvailable[i].extensionName);
    }

    GFX_FREE(pAvailable);

    return found;
}

void gfxCreateInstance(uint32_t apiVersion, uint32_t instanceExtensionCount, const char** ppInstanceExtensions)
{
    gfxInstance.apiVersion = apiVersion;

    // VK_EXT_debug_utils names objects and regions of command buffers for
    // debuggers and profilers, so it is enabled whenever it is available
    const char** ppExtensions = GFX_MALLOC((instanceExtensionCount + 1) * sizeof *ppExtensions);
    if (instanceExtensionCount) {
        memcpy(ppExtensions, ppInstanceExtensions, instanceExtensionCount * sizeof *ppExtensions);
    }
    uint32_t extensionCount = instanceExtensionCount;
    if (!extensionInList(extensionCount, ppExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) &&
        instanceExtensionAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        ppExtensions[extensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
    gfxInstance.debugUtils = extensionInList(extensionCount, ppExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VkApplicationInfo ai = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pEngineName = "GFX",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .apiVersion = apiVersion,
    };

    VkInstanceCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &ai,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = ppExtensions,
    };

#ifndef NDEBUG
    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
    ci.enabledLayerCount = GFX_ARRAY_LEN(layers);
    ci.ppEnabledLayerNames = layers;
#endif

    VK_CHECK(vkCreateInstance(&ci, NULL, &gfxInstance.instance));

    GFX_FREE(ppExtensions);

    uint32_t version;
    VK_CHECK(vkEnumerateInstanceVersion(&version));
    GFX_INFO("Created Vulkan instance: %d.%d.%d", VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version),
             VK_API_VERSION_PATCH(version));

#ifndef NDEBUG
    // Validation messages are reported through the debug utils messenger
    if (gfxInstance.debugUtils) {
        createDebugMessenger();
    }
#endif

    // Required before any use of the glslang C interface
    glslang_initialize_process();
}

void gfxDestroyInstance()
{
    if (gfxInstance.instance) {
#ifndef NDEBUG
        if (gfxInstance.debugMessenger) {
            destroyDebugUtilsMessengerEXT(gfxInstance.instance, gfxInstance.debugMessenger, NULL);
        }
#endif
        vkDestroyInstance(gfxInstance.instance, NULL);
    }

    glslang_finalize_process();

    GFX_RESET(&gfxInstance);
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

static bool deviceExtensionAvailable(VkPhysicalDevice physicalDevice, const char* pName)
{
    uint32_t n;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &n, NULL));
    VkExtensionProperties* pAvailable = GFX_MALLOC(n * sizeof *pAvailable);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &n, pAvailable));

    bool found = false;
    for (uint32_t i = 0; i < n && !found; i++) {
        found = !strcmp(pName, pAvailable[i].extensionName);
    }

    GFX_FREE(pAvailable);

    return found;
}

static const void* findStructure(const void* pNext, VkStructureType sType)
{
    for (const VkBaseInStructure* p = pNext; p; p = p->pNext) {
        if (p->sType == sType) {
            return p;
        }
    }
    return NULL;
}

// A structure type may only appear once in a pNext chain, so features that
// yggdrasil needs are switched on in the caller's structure when there is one,
// and otherwise linked into the chain. Feature structures are a header followed
// by VkBool32 members only, which lets them be merged without knowing the type.
static void mergeFeatures(VkDeviceCreateInfo* pCreateInfo, void* pRequired, size_t size)
{
    VkBaseOutStructure* pHeader = pRequired;
    VkBaseOutStructure* pExisting = (VkBaseOutStructure*)findStructure(pCreateInfo->pNext, pHeader->sType);

    if (pExisting) {
        VkBool32* pDst = (VkBool32*)(pExisting + 1);
        const VkBool32* pSrc = (const VkBool32*)(pHeader + 1);
        for (size_t i = 0; i < (size - sizeof *pHeader) / sizeof(VkBool32); i++) {
            pDst[i] |= pSrc[i];
        }
    } else {
        pHeader->pNext = (VkBaseOutStructure*)pCreateInfo->pNext;
        pCreateInfo->pNext = pHeader;
    }
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
    for (; index < n; index++) {
        if ((pProps[index].queueFlags & requiredFamilyFlags) == requiredFamilyFlags) {
            break;
        }
    }

    if (index == n) {
        GFX_FREE(pProps);
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

// Whether gfxCreateDevice() would accept a physical device, checked the same
// way it checks the chosen one
static bool deviceSupported(VkPhysicalDevice physicalDevice, uint32_t apiVersion, VkSurfaceKHR surface)
{
    if (apiVersion < VK_API_VERSION_1_3 ||
        !deviceExtensionAvailable(physicalDevice, VK_EXT_SHADER_OBJECT_EXTENSION_NAME) ||
        (surface && !deviceExtensionAvailable(physicalDevice, VK_KHR_SWAPCHAIN_EXTENSION_NAME))) {
        return false;
    }

    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
    };

    VkPhysicalDeviceFeatures2 features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &shaderObjectFeatures,
    };

    vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
    if (!shaderObjectFeatures.shaderObject) {
        return false;
    }

    // The device uses the first queue family that has graphics, compute and
    // transfer, which also has to present
    uint32_t n;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &n, NULL);
    VkQueueFamilyProperties* pProps = GFX_MALLOC(n * sizeof *pProps);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &n, pProps);

    const VkQueueFlags required = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    uint32_t index = 0;
    while (index < n && (pProps[index].queueFlags & required) != required) {
        index++;
    }

    GFX_FREE(pProps);

    if (index == n) {
        return false;
    }

    VkBool32 present = VK_TRUE;
    if (surface) {
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &present));
    }

    return present;
}

void gfxEnumerateDevices(VkSurfaceKHR surface, uint32_t* pDeviceCount, GfxDeviceInfo* pDevices)
{
    if (!gfxInstance.instance) {
        GFX_ERROR("Instance not initialized");
    }

    uint32_t n;
    VK_CHECK(vkEnumeratePhysicalDevices(gfxInstance.instance, &n, NULL));

    if (!pDevices) {
        *pDeviceCount = n;
        return;
    }

    VkPhysicalDevice* pPhysicalDevices = GFX_MALLOC(n * sizeof *pPhysicalDevices);
    VK_CHECK(vkEnumeratePhysicalDevices(gfxInstance.instance, &n, pPhysicalDevices));

    n = GFX_MIN(n, *pDeviceCount);
    for (uint32_t i = 0; i < n; i++) {
        VkPhysicalDeviceIDProperties id = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES,
        };

        VkPhysicalDeviceProperties2 prop = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &id,
        };

        vkGetPhysicalDeviceProperties2(pPhysicalDevices[i], &prop);

        GfxDeviceInfo* pInfo = &pDevices[i];
        memcpy(pInfo->name, prop.properties.deviceName, sizeof pInfo->name);
        memcpy(pInfo->uuid, id.deviceUUID, sizeof pInfo->uuid);
        pInfo->type = prop.properties.deviceType;
        pInfo->apiVersion = prop.properties.apiVersion;
        pInfo->supported = deviceSupported(pPhysicalDevices[i], prop.properties.apiVersion, surface);
    }

    GFX_FREE(pPhysicalDevices);

    *pDeviceCount = n;
}

void gfxCreateDevice(uint32_t physicalDeviceIndex, uint32_t deviceExtensionCount, const char** ppDeviceExtensions,
                     VkPhysicalDeviceFeatures2* features, VkSurfaceKHR surface)
{
    if (!gfxInstance.instance) {
        GFX_ERROR("Instance not initialized");
    }

    if (gfxDevice.device) {
        GFX_ERROR("Device already created");
    }

    gfxDevice.surface = surface;

    // Iterate all physical devices
    uint32_t n;
    VK_CHECK(vkEnumeratePhysicalDevices(gfxInstance.instance, &n, NULL));
    if (!n) {
        GFX_ERROR("No Vulkan devices available");
        return;
    }

    VkPhysicalDevice* pPhysicalDevices = GFX_MALLOC(n * sizeof *pPhysicalDevices);
    VK_CHECK(vkEnumeratePhysicalDevices(gfxInstance.instance, &n, pPhysicalDevices));

    if (physicalDeviceIndex == GFX_DEFAULT_DEVICE) {
        // Prefer the first supported discrete GPU, and settle for the first
        // supported device. If none is supported, the first device is tried so
        // that the checks below report why.
        uint32_t firstSupported = UINT32_MAX;
        physicalDeviceIndex = UINT32_MAX;
        for (uint32_t i = 0; i < n && physicalDeviceIndex == UINT32_MAX; i++) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(pPhysicalDevices[i], &properties);
            if (!deviceSupported(pPhysicalDevices[i], properties.apiVersion, surface)) {
                continue;
            }
            if (firstSupported == UINT32_MAX) {
                firstSupported = i;
            }
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                physicalDeviceIndex = i;
            }
        }
        if (physicalDeviceIndex == UINT32_MAX) {
            physicalDeviceIndex = firstSupported == UINT32_MAX ? 0 : firstSupported;
        }
    } else if (physicalDeviceIndex >= n) {
        GFX_ERROR("Device index %" PRIu32 " requested, but there are only %" PRIu32 " devices", physicalDeviceIndex, n);
        GFX_FREE(pPhysicalDevices);
        return;
    }

    GFX_INFO("Available devices (%d):", n);
    for (uint32_t i = 0; i < n; i++) {
        VkPhysicalDeviceAccelerationStructurePropertiesKHR asp = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR,
        };

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtp = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR,
            .pNext = &asp,
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
            gfxDevice.properties.rayTracingPipeline.pNext = NULL;
            gfxDevice.properties.accelerationStructure = asp;
            gfxDevice.properties.accelerationStructure.pNext = NULL;
        }

        GFX_INFO(" * [%d] %s, driver: %s %s, Vulkan %d.%d.%d %s", i, prop.properties.deviceName, driver.driverName,
                 driver.driverInfo, VK_API_VERSION_MAJOR(prop.properties.apiVersion),
                 VK_API_VERSION_MINOR(prop.properties.apiVersion), VK_API_VERSION_PATCH(prop.properties.apiVersion),
                 i == physicalDeviceIndex ? "(chosen)" : "");
    }

    GFX_FREE(pPhysicalDevices);

    uint32_t apiVersion = gfxDevice.properties.physicalDevice.apiVersion;
    if (apiVersion < VK_API_VERSION_1_3) {
        GFX_ERROR("The chosen device supports Vulkan %d.%d, but yggdrasil needs Vulkan 1.3",
                  VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion));
        return;
    }

    // Ray tracing is enabled whenever the device supports it
    bool rayTracingSupport =
        deviceExtensionAvailable(gfxDevice.physicalDevice, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) &&
        deviceExtensionAvailable(gfxDevice.physicalDevice, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) &&
        deviceExtensionAvailable(gfxDevice.physicalDevice, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineSupport = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
    };

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureSupport = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        .pNext = &rayTracingPipelineSupport,
    };

    VkPhysicalDeviceFeatures2 support = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = rayTracingSupport ? &accelerationStructureSupport : NULL,
    };

    vkGetPhysicalDeviceFeatures2(gfxDevice.physicalDevice, &support);
    rayTracingSupport = rayTracingSupport && accelerationStructureSupport.accelerationStructure &&
                        rayTracingPipelineSupport.rayTracingPipeline;

    // The extensions that yggdrasil uses, followed by the requested ones that
    // are not already in the list
    const char* ppRequiredExtensions[5];
    uint32_t requiredExtensionCount = 0;
    ppRequiredExtensions[requiredExtensionCount++] = VK_EXT_SHADER_OBJECT_EXTENSION_NAME;
    if (surface) {
        ppRequiredExtensions[requiredExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    }
    if (rayTracingSupport) {
        ppRequiredExtensions[requiredExtensionCount++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
        ppRequiredExtensions[requiredExtensionCount++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
        ppRequiredExtensions[requiredExtensionCount++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
    }

    uint32_t extensionCount = 0;
    const char** ppExtensions = GFX_MALLOC((requiredExtensionCount + deviceExtensionCount) * sizeof *ppExtensions);
    for (uint32_t i = 0; i < requiredExtensionCount + deviceExtensionCount; i++) {
        const char* pName =
            i < requiredExtensionCount ? ppRequiredExtensions[i] : ppDeviceExtensions[i - requiredExtensionCount];
        if (!extensionInList(extensionCount, ppExtensions, pName)) {
            ppExtensions[extensionCount++] = pName;
        }
    }

    if (!checkDeviceExtensionSupport(extensionCount, ppExtensions)) {
        GFX_FREE(ppExtensions);
        return;
    }

    // Entry points of extensions that are not enabled are not reliably NULL,
    // so what is enabled is taken from the extension list
    gfxDevice.accelerationStructures =
        extensionInList(extensionCount, ppExtensions, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    gfxDevice.rayTracingPipelines =
        extensionInList(extensionCount, ppExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

    uint32_t queueFamilyIndex =
        getQueueFamilyIndex(surface, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

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
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = ppExtensions,
    };

    // The features that yggdrasil uses, merged with the requested ones
    VkPhysicalDeviceFeatures2 requiredFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .features =
            {
                .samplerAnisotropy = support.features.samplerAnisotropy,
            },
    };

    VkPhysicalDeviceVulkan12Features requiredVk12Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .bufferDeviceAddress = VK_TRUE,
    };

    // Shaders are compiled to SPIR-V 1.6, where discard demotes the invocation
    // to a helper invocation
    VkPhysicalDeviceVulkan13Features requiredVk13Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .shaderDemoteToHelperInvocation = VK_TRUE,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceShaderObjectFeaturesEXT requiredShaderObjectFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
        .shaderObject = VK_TRUE,
    };

    VkPhysicalDeviceAccelerationStructureFeaturesKHR requiredAccelerationStructureFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        .accelerationStructure = VK_TRUE,
    };

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR requiredRayTracingPipelineFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .rayTracingPipeline = VK_TRUE,
    };

    mergeFeatures(&ci, &requiredFeatures, sizeof requiredFeatures);
    mergeFeatures(&ci, &requiredVk12Features, sizeof requiredVk12Features);
    mergeFeatures(&ci, &requiredVk13Features, sizeof requiredVk13Features);
    mergeFeatures(&ci, &requiredShaderObjectFeatures, sizeof requiredShaderObjectFeatures);
    if (gfxDevice.accelerationStructures) {
        mergeFeatures(&ci, &requiredAccelerationStructureFeatures, sizeof requiredAccelerationStructureFeatures);
    }
    if (gfxDevice.rayTracingPipelines) {
        mergeFeatures(&ci, &requiredRayTracingPipelineFeatures, sizeof requiredRayTracingPipelineFeatures);
    }

    VK_CHECK(vkCreateDevice(gfxDevice.physicalDevice, &ci, NULL, &gfxDevice.device));

    GFX_FREE(ppExtensions);

    const VkPhysicalDeviceFeatures2* pFeatures = findStructure(ci.pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);
    const VkPhysicalDeviceMeshShaderFeaturesEXT* pMeshFeatures =
        findStructure(ci.pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT);

    // Remember whether anisotropic filtering is enabled, so that samplers are
    // not created with a feature that the device was not created with
    gfxDevice.samplerAnisotropy = pFeatures->features.samplerAnisotropy;

    // Remember which graphics stages exist on the device, so that shaders can
    // bind the ones they do not use to null
    gfxDevice.graphicsShaderStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    if (pFeatures->features.tessellationShader) {
        gfxDevice.graphicsShaderStages |=
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    }
    if (pFeatures->features.geometryShader) {
        gfxDevice.graphicsShaderStages |= VK_SHADER_STAGE_GEOMETRY_BIT;
    }
    if (pMeshFeatures && pMeshFeatures->taskShader) {
        gfxDevice.graphicsShaderStages |= VK_SHADER_STAGE_TASK_BIT_EXT;
    }
    if (pMeshFeatures && pMeshFeatures->meshShader) {
        gfxDevice.graphicsShaderStages |= VK_SHADER_STAGE_MESH_BIT_EXT;
    }

    // Resolve extension entry points once, rather than on every call
#define GFX_LOAD_FN(name) gfxDevice.fn.name = (PFN_##name)vkGetDeviceProcAddr(gfxDevice.device, #name)
    GFX_LOAD_FN(vkCreateShadersEXT);
    GFX_LOAD_FN(vkDestroyShaderEXT);
    GFX_LOAD_FN(vkCmdBindShadersEXT);
    GFX_LOAD_FN(vkCmdSetVertexInputEXT);
    GFX_LOAD_FN(vkCmdSetRasterizationSamplesEXT);
    GFX_LOAD_FN(vkCmdSetSampleMaskEXT);
    GFX_LOAD_FN(vkCmdSetAlphaToCoverageEnableEXT);
    GFX_LOAD_FN(vkCmdSetPolygonModeEXT);
    GFX_LOAD_FN(vkCmdSetLogicOpEnableEXT);
    GFX_LOAD_FN(vkCmdSetColorBlendEnableEXT);
    GFX_LOAD_FN(vkCmdSetColorWriteMaskEXT);
    GFX_LOAD_FN(vkCmdSetColorBlendEquationEXT);
    GFX_LOAD_FN(vkCreateAccelerationStructureKHR);
    GFX_LOAD_FN(vkDestroyAccelerationStructureKHR);
    GFX_LOAD_FN(vkGetAccelerationStructureBuildSizesKHR);
    GFX_LOAD_FN(vkGetAccelerationStructureDeviceAddressKHR);
    GFX_LOAD_FN(vkCmdBuildAccelerationStructuresKHR);
    GFX_LOAD_FN(vkCreateRayTracingPipelinesKHR);
    GFX_LOAD_FN(vkGetRayTracingShaderGroupHandlesKHR);
    GFX_LOAD_FN(vkCmdTraceRaysKHR);
#undef GFX_LOAD_FN

    // Debug utils is an instance extension, so its entry points come from the
    // instance
    if (gfxInstance.debugUtils) {
#define GFX_LOAD_FN(name) gfxDevice.fn.name = (PFN_##name)vkGetInstanceProcAddr(gfxInstance.instance, #name)
        GFX_LOAD_FN(vkSetDebugUtilsObjectNameEXT);
        GFX_LOAD_FN(vkCmdBeginDebugUtilsLabelEXT);
        GFX_LOAD_FN(vkCmdEndDebugUtilsLabelEXT);
#undef GFX_LOAD_FN
    }

    gfxDevice.queueFamilyIndex = queueFamilyIndex;

    // Timestamps hold timestampValidBits bits, and the rest is to be ignored
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gfxDevice.physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* pQueueFamilies = GFX_MALLOC(queueFamilyCount * sizeof *pQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(gfxDevice.physicalDevice, &queueFamilyCount, pQueueFamilies);
    uint32_t validBits = pQueueFamilies[queueFamilyIndex].timestampValidBits;
    gfxDevice.timestampMask = validBits >= 64 ? UINT64_MAX : (1ull << validBits) - 1;
    GFX_FREE(pQueueFamilies);

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

    destroyUploadBatches();
    vkDeviceWaitIdle(gfxDevice.device);
    destroyMemoryBlocks();

    if (gfxDevice.commandPool) {
        vkDestroyCommandPool(gfxDevice.device, gfxDevice.commandPool, NULL);
    }
    if (gfxDevice.device) {
        vkDestroyDevice(gfxDevice.device, NULL);
    }
    if (gfxDevice.surface) {
        vkDestroySurfaceKHR(gfxInstance.instance, gfxDevice.surface, NULL);
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
        GFX_MALLOC(gfxSwapchain.supportDetails.presentCount * sizeof *gfxSwapchain.supportDetails.presentModes);
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

// Vertical synchronization uses VK_PRESENT_MODE_FIFO_KHR, which is the only
// mode that is always available. Without it, VK_PRESENT_MODE_IMMEDIATE_KHR is
// preferred, then VK_PRESENT_MODE_MAILBOX_KHR, which at least does not wait for
// vertical blank, and FIFO as a last resort.
static VkPresentModeKHR choosePresentMode(uint32_t availablePresentModeCount,
                                          const VkPresentModeKHR* availablePresentModes, bool vsync)
{
    if (vsync) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    const VkPresentModeKHR preferred[] = {VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR};
    for (uint32_t i = 0; i < GFX_ARRAY_LEN(preferred); i++) {
        for (uint32_t j = 0; j < availablePresentModeCount; j++) {
            if (availablePresentModes[j] == preferred[i]) {
                return preferred[i];
            }
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

        char name[32];
        snprintf(name, sizeof name, "Swapchain image %" PRIu32, i);
        gfxSetDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)gfxSwapchain.images[i], name);
        gfxSetDebugName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)gfxSwapchain.imageViews[i], name);
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

        char name[32];
        snprintf(name, sizeof name, "Frame %" PRIu32, i);
        gfxSetDebugName(VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)(uintptr_t)gfxSwapchain.commandBuffers[i], name);
    }
}

// The timers of a frame in flight live as long as the swapchain, also when it
// is recreated, since the number of frames in flight stays the same
static void createTimers()
{
    if (!gfxDevice.timestampMask) {
        GFX_WARNING("The queue does not support timestamps, so there are no GPU timers");
        return;
    }

    gfxTimers.pFrames = GFX_MALLOC(gfxSwapchain.framesInFlight * sizeof *gfxTimers.pFrames);

    VkQueryPoolCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount = 2 * GFX_MAX_TIMERS,
    };

    for (uint32_t i = 0; i < gfxSwapchain.framesInFlight; i++) {
        GFX_RESET(&gfxTimers.pFrames[i]);
        VK_CHECK(vkCreateQueryPool(gfxDevice.device, &ci, NULL, &gfxTimers.pFrames[i].queryPool));
        gfxSetDebugName(VK_OBJECT_TYPE_QUERY_POOL, (uint64_t)gfxTimers.pFrames[i].queryPool, "GPU timers");
    }
}

static void destroyTimers()
{
    if (gfxTimers.pFrames) {
        for (uint32_t i = 0; i < gfxSwapchain.framesInFlight; i++) {
            vkDestroyQueryPool(gfxDevice.device, gfxTimers.pFrames[i].queryPool, NULL);
        }
        GFX_FREE(gfxTimers.pFrames);
    }

    GFX_RESET(&gfxTimers);
}

// Read back the timers of the frame in flight, whose fence has been waited on,
// so that the frame that last used its queries has finished
static void readTimers()
{
    if (!gfxTimers.pFrames) {
        return;
    }

    GfxTimerFrame* pFrame = &gfxTimers.pFrames[gfxSwapchain.inFlightIndex];

    if (pFrame->count) {
        uint64_t timestamps[2 * GFX_MAX_TIMERS];
        VkResult result = vkGetQueryPoolResults(gfxDevice.device, pFrame->queryPool, 0, 2 * pFrame->count,
                                                2 * pFrame->count * sizeof *timestamps, timestamps, sizeof *timestamps,
                                                VK_QUERY_RESULT_64_BIT);

        if (result == VK_SUCCESS) {
            // The timestamp period is in nanoseconds per tick
            double period = gfxDevice.properties.physicalDevice.limits.timestampPeriod;
            for (uint32_t i = 0; i < pFrame->count; i++) {
                uint64_t ticks = (timestamps[2 * i + 1] - timestamps[2 * i]) & gfxDevice.timestampMask;
                gfxTimers.results[i] = pFrame->timers[i];
                gfxTimers.results[i].milliseconds = (double)ticks * period * 1e-6;
            }
            gfxTimers.resultCount = pFrame->count;
        }
    }

    pFrame->count = 0;
    gfxTimers.depth = 0;
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
    createTimers();
}

void gfxDestroySwapchain()
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    vkDeviceWaitIdle(gfxDevice.device);

    destroyTimers();
    destroySyncObjects();
    destroySwapchain();

    GFX_FREE(gfxSwapchain.supportDetails.formats);
    GFX_FREE(gfxSwapchain.supportDetails.presentModes);

    GFX_RESET(&gfxSwapchain);
}

void gfxSetVsync(bool vsync)
{
    if (gfxDevice.vsync == vsync) {
        return;
    }

    gfxDevice.vsync = vsync;

    // Present mode is baked into the swapchain, so it has to be rebuilt
    if (gfxSwapchain.swapchain) {
        gfxRecreateSwapchain();
    }
}

void gfxRecreateSwapchain()
{
    uint32_t width, height;
    gfxSwapchain.framebufferSizeCallback(&width, &height);

    GFX_DEBUG("Recreating swapchain %" PRIu32 "x%" PRIu32, width, height);

    destroySyncObjects();
    destroySwapchain();

    // The device has been waited on, so every frame recorded before has finished
    gfxDevice.completedFrames = gfxDevice.frame;

    GFX_FREE(gfxSwapchain.supportDetails.formats);
    GFX_FREE(gfxSwapchain.supportDetails.presentModes);

    querySupport();
    createSwapchain();
    createSyncObjects();

    gfxSwapchain.recreated = true;
}

void gfxWaitForFence()
{
    VK_CHECK(vkWaitForFences(gfxDevice.device, 1, &gfxSwapchain.inFlightFences[gfxSwapchain.inFlightIndex], VK_TRUE,
                             UINT64_MAX));

    // The fence was signaled by the frame that last used this slot, so that
    // frame and every frame before it have finished
    if (gfxDevice.frame >= gfxSwapchain.framesInFlight) {
        gfxDevice.completedFrames =
            GFX_MAX(gfxDevice.completedFrames, gfxDevice.frame - gfxSwapchain.framesInFlight + 1);
    }
}

VkCommandBuffer gfxAcquireNextImage()
{
    // The caller has had a full iteration to react to a recreated swapchain
    gfxSwapchain.recreated = false;

    // Drivers are allowed to keep presenting to a resized window instead of
    // reporting the swapchain out of date, so the size is compared here. A
    // surface without a fixed size reports UINT32_MAX, and a minimized window
    // reports zero, and neither is a reason to recreate.
    VkSurfaceCapabilitiesKHR capabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfxDevice.physicalDevice, gfxDevice.surface, &capabilities));
    VkExtent2D currentExtent = capabilities.currentExtent;
    if (currentExtent.width != UINT32_MAX && currentExtent.width && currentExtent.height &&
        (currentExtent.width != gfxSwapchain.extent.width || currentExtent.height != gfxSwapchain.extent.height)) {
        gfxRecreateSwapchain();
    }

    // Wait for the current frame to not be in flight
    gfxWaitForFence();
    readTimers();
    VK_CHECK(vkResetFences(gfxDevice.device, 1, &gfxSwapchain.inFlightFences[gfxSwapchain.inFlightIndex]));

    // Acquire index of next image in the swapchain
    VkResult result = vkAcquireNextImageKHR(gfxDevice.device, gfxSwapchain.swapchain, UINT64_MAX,
                                            gfxSwapchain.inFlightSemaphores[gfxSwapchain.inFlightIndex], NULL,
                                            &gfxSwapchain.imageIndex);

    // Check if swapchain needs to be reconstructed. No image was acquired and
    // no semaphore was signaled, so this frame cannot be rendered.
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        gfxRecreateSwapchain();
        return VK_NULL_HANDLE;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        GFX_ERROR("Failed to acquire next swapchain image");
    }

    VkCommandBufferBeginInfo bi = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkCommandBuffer cmd = gfxSwapchain.commandBuffers[gfxSwapchain.inFlightIndex];
    VK_CHECK(vkBeginCommandBuffer(cmd, &bi));

    if (gfxTimers.pFrames) {
        vkCmdResetQueryPool(cmd, gfxTimers.pFrames[gfxSwapchain.inFlightIndex].queryPool, 0, 2 * GFX_MAX_TIMERS);
    }
    gfxCmdBeginTimer(cmd, "Frame");

    return cmd;
}

void gfxPresent(VkCommandBuffer cmd, GfxImage* pImage)
{
    // Transition swapchain image for blitting. The image comes from the
    // presentation engine, so the acquire semaphore provides the source
    // dependency.
    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    gfxSwapchain.images[gfxSwapchain.imageIndex], NULL);

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

    // Every timer has to be ended for the frame's timers to be read back, so
    // timers left running end here, together with the frame's own
    if (gfxTimers.depth > 1) {
        GFX_WARNING("%" PRIu32 " GPU timers were not ended", gfxTimers.depth - 1);
    }
    while (gfxTimers.depth) {
        gfxCmdEndTimer(cmd);
    }

    VK_CHECK(vkEndCommandBuffer(cmd));

    // Uploads recorded until now are submitted ahead of the frame, which may
    // use them
    gfxFlushUploads(false);

    // The first use of the acquired image is the blit above, so the acquire
    // semaphore has to be waited on before the transfer stage
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

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
    gfxDevice.frame++;
}

// Memory blocks //

// Allocate device memory, mapped if it is host visible. Memory for buffers can
// hold buffers with device addresses, which every block allows since any
// buffer may end up in it.
static VkDeviceMemory allocateDeviceMemory(VkDeviceSize size, uint32_t memoryTypeIndex, bool deviceAddress,
                                           const void* pNext, void** ppMapped)
{
    VkMemoryAllocateFlagsInfo flagsInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .pNext = pNext,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
    };

    VkMemoryAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = deviceAddress ? &flagsInfo : pNext,
        .allocationSize = size,
        .memoryTypeIndex = memoryTypeIndex,
    };

    VkDeviceMemory memory;
    VK_CHECK(vkAllocateMemory(gfxDevice.device, &ai, NULL, &memory));

    *ppMapped = NULL;
    if (gfxDevice.properties.memory.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        VK_CHECK(vkMapMemory(gfxDevice.device, memory, 0, VK_WHOLE_SIZE, 0, ppMapped));
    }

    return memory;
}

static GfxMemoryBlock* findMemoryBlock(VkDeviceMemory memory)
{
    for (uint32_t i = 0; i < gfxMemory.blockCount; i++) {
        if (gfxMemory.pBlocks[i].memory == memory) {
            return &gfxMemory.pBlocks[i];
        }
    }
    return NULL;
}

// Replace removeCount free ranges of a block, from index on, with insertCount
// others
static void spliceFreeRanges(GfxMemoryBlock* pBlock, uint32_t index, uint32_t removeCount,
                             const GfxMemoryRange* pInsert, uint32_t insertCount)
{
    uint32_t count = pBlock->freeCount - removeCount + insertCount;
    if (count > pBlock->freeCapacity) {
        pBlock->freeCapacity = GFX_MAX(count, pBlock->freeCapacity * 2);
        pBlock->pFree = GFX_REALLOC(pBlock->pFree, pBlock->freeCapacity * sizeof *pBlock->pFree);
    }

    memmove(&pBlock->pFree[index + insertCount], &pBlock->pFree[index + removeCount],
            (pBlock->freeCount - index - removeCount) * sizeof *pBlock->pFree);
    memcpy(&pBlock->pFree[index], pInsert, insertCount * sizeof *pInsert);
    pBlock->freeCount = count;
}

// Take the first free range that fits, leaving what is before and after it
// free
static bool allocateFromBlock(GfxMemoryBlock* pBlock, VkDeviceSize size, VkDeviceSize alignment, VkDeviceSize* pOffset)
{
    for (uint32_t i = 0; i < pBlock->freeCount; i++) {
        GfxMemoryRange range = pBlock->pFree[i];
        VkDeviceSize start = gfxAlignTo(range.offset, alignment);
        if (start + size > range.offset + range.size) {
            continue;
        }

        GfxMemoryRange remaining[2];
        uint32_t remainingCount = 0;
        if (start > range.offset) {
            remaining[remainingCount++] = (GfxMemoryRange){range.offset, start - range.offset};
        }
        if (start + size < range.offset + range.size) {
            remaining[remainingCount++] = (GfxMemoryRange){start + size, range.offset + range.size - start - size};
        }
        spliceFreeRanges(pBlock, i, 1, remaining, remainingCount);

        pBlock->allocationCount++;
        *pOffset = start;
        return true;
    }
    return false;
}

static void destroyMemoryBlock(uint32_t index)
{
    GfxMemoryBlock* pBlock = &gfxMemory.pBlocks[index];
    vkFreeMemory(gfxDevice.device, pBlock->memory, NULL);
    GFX_FREE(pBlock->pFree);

    gfxMemory.pBlocks[index] = gfxMemory.pBlocks[--gfxMemory.blockCount];
}

// Find memory for a resource, in a block of the right memory type that holds
// the same kind of resources, in a new block, or in memory of its own. Buffers
// and images with optimal tiling never share a block, so that
// bufferImageGranularity does not have to be followed.
static GfxMemoryAllocation allocateMemory(const VkMemoryRequirements* pRequirements, VkMemoryPropertyFlags properties,
                                          bool optimal, bool dedicated, const void* pDedicatedInfo)
{
    uint32_t memoryTypeIndex = gfxFindMemoryType(pRequirements->memoryTypeBits, properties);
    uint32_t heapIndex = gfxDevice.properties.memory.memoryTypes[memoryTypeIndex].heapIndex;
    VkDeviceSize blockSize =
        GFX_MIN((VkDeviceSize)GFX_MEMORY_BLOCK_SIZE, gfxDevice.properties.memory.memoryHeaps[heapIndex].size / 8);

    GfxMemoryAllocation allocation = {.size = pRequirements->size};

    if (dedicated || pRequirements->size > blockSize / 2) {
        allocation.memory =
            allocateDeviceMemory(pRequirements->size, memoryTypeIndex, !optimal, pDedicatedInfo, &allocation.pMapped);
        gfxMemory.dedicatedCount++;
        return allocation;
    }

    for (uint32_t i = 0; i < gfxMemory.blockCount; i++) {
        GfxMemoryBlock* pBlock = &gfxMemory.pBlocks[i];
        if (pBlock->memoryTypeIndex == memoryTypeIndex && pBlock->optimal == optimal &&
            allocateFromBlock(pBlock, pRequirements->size, pRequirements->alignment, &allocation.offset)) {
            allocation.memory = pBlock->memory;
            allocation.pMapped = pBlock->pMapped ? (char*)pBlock->pMapped + allocation.offset : NULL;
            return allocation;
        }
    }

    if (gfxMemory.blockCount == gfxMemory.blockCapacity) {
        gfxMemory.blockCapacity = gfxMemory.blockCapacity ? gfxMemory.blockCapacity * 2 : 8;
        gfxMemory.pBlocks = GFX_REALLOC(gfxMemory.pBlocks, gfxMemory.blockCapacity * sizeof *gfxMemory.pBlocks);
    }

    GfxMemoryBlock* pBlock = &gfxMemory.pBlocks[gfxMemory.blockCount++];
    *pBlock = (GfxMemoryBlock){
        .size = blockSize,
        .memoryTypeIndex = memoryTypeIndex,
        .optimal = optimal,
    };
    pBlock->memory = allocateDeviceMemory(blockSize, memoryTypeIndex, !optimal, NULL, &pBlock->pMapped);
    spliceFreeRanges(pBlock, 0, 0, &(GfxMemoryRange){0, blockSize}, 1);

    char name[64];
    snprintf(name, sizeof name, "Memory block, type %" PRIu32 "%s", memoryTypeIndex, optimal ? ", images" : "");
    gfxSetDebugName(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)pBlock->memory, name);
    GFX_DEBUG("Allocated a %" PRIu64 " MB memory block of type %" PRIu32 "%s", (uint64_t)(blockSize >> 20),
              memoryTypeIndex, optimal ? " for images" : "");

    allocateFromBlock(pBlock, pRequirements->size, pRequirements->alignment, &allocation.offset);
    allocation.memory = pBlock->memory;
    allocation.pMapped = pBlock->pMapped ? (char*)pBlock->pMapped + allocation.offset : NULL;
    return allocation;
}

// Give memory back to its block, merging it with the free ranges around it.
// Of the blocks that end up empty, one of each kind is kept for later use.
static void freeMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size)
{
    if (!memory) {
        return;
    }

    GfxMemoryBlock* pBlock = findMemoryBlock(memory);
    if (!pBlock) {
        vkFreeMemory(gfxDevice.device, memory, NULL);
        gfxMemory.dedicatedCount--;
        return;
    }

    uint32_t next = 0;
    while (next < pBlock->freeCount && pBlock->pFree[next].offset < offset) {
        next++;
    }

    GfxMemoryRange merged = {offset, size};
    uint32_t index = next;
    uint32_t removeCount = 0;
    if (next > 0 && pBlock->pFree[next - 1].offset + pBlock->pFree[next - 1].size == offset) {
        index = next - 1;
        merged.offset = pBlock->pFree[index].offset;
        merged.size += pBlock->pFree[index].size;
        removeCount++;
    }
    if (next < pBlock->freeCount && offset + size == pBlock->pFree[next].offset) {
        merged.size += pBlock->pFree[next].size;
        removeCount++;
    }
    spliceFreeRanges(pBlock, index, removeCount, &merged, 1);
    pBlock->allocationCount--;

    if (pBlock->allocationCount) {
        return;
    }

    for (uint32_t i = 0; i < gfxMemory.blockCount; i++) {
        GfxMemoryBlock* pOther = &gfxMemory.pBlocks[i];
        if (pOther != pBlock && !pOther->allocationCount && pOther->memoryTypeIndex == pBlock->memoryTypeIndex &&
            pOther->optimal == pBlock->optimal) {
            destroyMemoryBlock((uint32_t)(pBlock - gfxMemory.pBlocks));
            return;
        }
    }
}

static void destroyMemoryBlocks()
{
    uint32_t leaked = gfxMemory.dedicatedCount;
    for (uint32_t i = 0; i < gfxMemory.blockCount; i++) {
        leaked += gfxMemory.pBlocks[i].allocationCount;
    }
    if (leaked) {
        GFX_WARNING("%" PRIu32 " buffers and images were not destroyed before the device", leaked);
    }

    while (gfxMemory.blockCount) {
        destroyMemoryBlock(gfxMemory.blockCount - 1);
    }

    GFX_FREE(gfxMemory.pBlocks);
    GFX_RESET(&gfxMemory);
}

void gfxCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, GfxBuffer* pBuffer)
{
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

    VkMemoryDedicatedRequirements dedicatedRequirements = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS,
    };
    VkMemoryRequirements2 requirements = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = &dedicatedRequirements,
    };
    VkBufferMemoryRequirementsInfo2 requirementsInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
        .buffer = pBuffer->buffer,
    };
    vkGetBufferMemoryRequirements2(gfxDevice.device, &requirementsInfo, &requirements);

    VkMemoryDedicatedAllocateInfo dedicatedInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
        .buffer = pBuffer->buffer,
    };
    GfxMemoryAllocation allocation = allocateMemory(&requirements.memoryRequirements, properties, false,
                                                    dedicatedRequirements.prefersDedicatedAllocation ||
                                                        dedicatedRequirements.requiresDedicatedAllocation,
                                                    &dedicatedInfo);

    pBuffer->size = requirements.memoryRequirements.size;
    pBuffer->memory = allocation.memory;
    pBuffer->memoryOffset = allocation.offset;
    pBuffer->memorySize = allocation.size;

    VK_CHECK(vkBindBufferMemory(gfxDevice.device, pBuffer->buffer, pBuffer->memory, pBuffer->memoryOffset));

    // Host coherent memory stays mapped
    if (pBuffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        pBuffer->pHostMap = allocation.pMapped;
    }
}

// Release a buffer that the device is known to be done with
static void releaseBuffer(GfxBuffer* pBuffer)
{
    vkDestroyBuffer(gfxDevice.device, pBuffer->buffer, NULL);
    freeMemory(pBuffer->memory, pBuffer->memoryOffset, pBuffer->memorySize);

    GFX_RESET(pBuffer);
}

void gfxDestroyBuffer(GfxBuffer* pBuffer)
{
    // A pending upload may still copy to the buffer or read it
    gfxFlushUploads(true);
    vkDeviceWaitIdle(gfxDevice.device);

    releaseBuffer(pBuffer);
    gfxDevice.resourceGeneration++;
}

// Upload batches //

static void finishUploadBatch(GfxUploadBatch* pBatch)
{
    for (uint32_t i = 0; i < pBatch->retiredCount; i++) {
        releaseBuffer(&pBatch->pRetired[i]);
    }
    pBatch->retiredCount = 0;

    VK_CHECK(vkResetFences(gfxDevice.device, 1, &pBatch->fence));
    pBatch->submitted = false;
}

// Finish the batches whose fences have signaled, or with wait, every submitted
// batch
static void recycleUploadBatches(bool wait)
{
    for (uint32_t i = 0; i < GFX_UPLOAD_BATCHES; i++) {
        GfxUploadBatch* pBatch = &gfxUploads.batches[i];
        if (!pBatch->submitted) {
            continue;
        }

        if (wait) {
            VK_CHECK(vkWaitForFences(gfxDevice.device, 1, &pBatch->fence, VK_TRUE, UINT64_MAX));
        } else if (vkGetFenceStatus(gfxDevice.device, pBatch->fence) != VK_SUCCESS) {
            continue;
        }

        finishUploadBatch(pBatch);
    }
}

// Get the command buffer of the batch being recorded, and begin recording one
// if there is none. A batch is reused once the one submitted
// GFX_UPLOAD_BATCHES batches ago has finished.
static VkCommandBuffer uploadCommandBuffer()
{
    GfxUploadBatch* pBatch = &gfxUploads.batches[gfxUploads.current];

    if (gfxUploads.recording) {
        return pBatch->cmd;
    }

    if (!pBatch->cmd) {
        VkCommandBufferAllocateInfo ai = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = gfxDevice.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VK_CHECK(vkAllocateCommandBuffers(gfxDevice.device, &ai, &pBatch->cmd));
        gfxSetDebugName(VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)(uintptr_t)pBatch->cmd, "Uploads");

        VkFenceCreateInfo fci = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        VK_CHECK(vkCreateFence(gfxDevice.device, &fci, NULL, &pBatch->fence));
    }

    if (pBatch->submitted) {
        VK_CHECK(vkWaitForFences(gfxDevice.device, 1, &pBatch->fence, VK_TRUE, UINT64_MAX));
        finishUploadBatch(pBatch);
    }

    VkCommandBufferBeginInfo bi = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VK_CHECK(vkBeginCommandBuffer(pBatch->cmd, &bi));

    pBatch->used = 0;
    pBatch->writtenCount = 0;
    pBatch->transferred = false;
    pBatch->built = false;
    gfxUploads.recording = true;

    gfxCmdBeginLabel(pBatch->cmd, "Uploads");

    // Work submitted before the batch, frames in flight included, may still use
    // what the batch writes
    gfxMemoryBarrier(pBatch->cmd, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);

    return pBatch->cmd;
}

static void submitUploadBatch()
{
    if (!gfxUploads.recording) {
        return;
    }

    GfxUploadBatch* pBatch = &gfxUploads.batches[gfxUploads.current];

    // Work submitted after the batch sees what it wrote. Barriers reach later
    // submissions to the same queue, so no semaphore is needed.
    gfxMemoryBarrier(pBatch->cmd, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);

    gfxCmdEndLabel(pBatch->cmd);
    VK_CHECK(vkEndCommandBuffer(pBatch->cmd));

    VkSubmitInfo si = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &pBatch->cmd,
    };
    VK_CHECK(vkQueueSubmit(gfxDevice.queue, 1, &si, pBatch->fence));

    pBatch->submitted = true;
    gfxUploads.recording = false;
    gfxUploads.current = (gfxUploads.current + 1) % GFX_UPLOAD_BATCHES;
}

void gfxFlushUploads(bool wait)
{
    if (!gfxDevice.device) {
        return;
    }

    submitUploadBatch();
    recycleUploadBatches(wait);
}

static void destroyUploadBatches()
{
    gfxFlushUploads(true);

    // The command buffers are freed with the command pool
    for (uint32_t i = 0; i < GFX_UPLOAD_BATCHES; i++) {
        GfxUploadBatch* pBatch = &gfxUploads.batches[i];
        if (pBatch->staging.buffer) {
            releaseBuffer(&pBatch->staging);
        }
        if (pBatch->fence) {
            vkDestroyFence(gfxDevice.device, pBatch->fence, NULL);
        }
        GFX_FREE(pBatch->pRetired);
        GFX_FREE(pBatch->pWritten);
    }

    GFX_RESET(&gfxUploads);
}

// Release a buffer when the batch being recorded has finished
static void retireBuffer(const GfxBuffer* pBuffer)
{
    GfxUploadBatch* pBatch = &gfxUploads.batches[gfxUploads.current];

    if (pBatch->retiredCount == pBatch->retiredCapacity) {
        pBatch->retiredCapacity = pBatch->retiredCapacity ? pBatch->retiredCapacity * 2 : 4;
        pBatch->pRetired = GFX_REALLOC(pBatch->pRetired, pBatch->retiredCapacity * sizeof *pBatch->pRetired);
    }

    pBatch->pRetired[pBatch->retiredCount++] = *pBuffer;
}

typedef struct GfxStaging {
    VkBuffer buffer;
    VkDeviceSize offset;
    void* pData;
} GfxStaging;

// Take staging memory in the batch being recorded, beginning a new batch if the
// current one has no room left. Copies to images need offsets that are
// multiples of the texel size, so offsets are multiples of both it and 16.
static GfxStaging allocateStaging(VkDeviceSize size, VkDeviceSize texelSize)
{
    VkDeviceSize alignment = 16;
    while (texelSize && alignment % texelSize) {
        alignment += 16;
    }

    uploadCommandBuffer();

    if (size > GFX_UPLOAD_STAGING_SIZE) {
        GfxBuffer buffer;
        gfxCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer);
        gfxSetBufferName(&buffer, "Upload staging");
        retireBuffer(&buffer);
        return (GfxStaging){.buffer = buffer.buffer, .offset = 0, .pData = buffer.pHostMap};
    }

    GfxUploadBatch* pBatch = &gfxUploads.batches[gfxUploads.current];
    if ((pBatch->used + alignment - 1) / alignment * alignment + size > GFX_UPLOAD_STAGING_SIZE) {
        submitUploadBatch();
        uploadCommandBuffer();
        pBatch = &gfxUploads.batches[gfxUploads.current];
    }

    if (!pBatch->staging.buffer) {
        gfxCreateBuffer(GFX_UPLOAD_STAGING_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &pBatch->staging);
        gfxSetBufferName(&pBatch->staging, "Upload staging");
    }

    VkDeviceSize offset = (pBatch->used + alignment - 1) / alignment * alignment;
    pBatch->used = offset + size;

    return (GfxStaging){
        .buffer = pBatch->staging.buffer,
        .offset = offset,
        .pData = (char*)pBatch->staging.pHostMap + offset,
    };
}

// Order a copy to a buffer in the batch being recorded after the builds that
// may read the buffer, and after earlier copies to the same buffer, which could
// otherwise run in any order
static void beforeUploadCopy(VkBuffer buffer)
{
    GfxUploadBatch* pBatch = &gfxUploads.batches[gfxUploads.current];

    if (pBatch->built) {
        gfxMemoryBarrier(pBatch->cmd, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_NONE,
                         VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);
        pBatch->built = false;
    }

    for (uint32_t i = 0; i < pBatch->writtenCount; i++) {
        if (pBatch->pWritten[i] == buffer) {
            gfxMemoryBarrier(pBatch->cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                             VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);
            pBatch->writtenCount = 0;
            break;
        }
    }

    if (pBatch->writtenCount == pBatch->writtenCapacity) {
        pBatch->writtenCapacity = pBatch->writtenCapacity ? pBatch->writtenCapacity * 2 : 16;
        pBatch->pWritten = GFX_REALLOC(pBatch->pWritten, pBatch->writtenCapacity * sizeof *pBatch->pWritten);
    }
    pBatch->pWritten[pBatch->writtenCount++] = buffer;
    pBatch->transferred = true;
}

// Order a build in the batch being recorded after the copies before it, which
// may have uploaded its geometry. Builds after each other are not ordered, so
// they can overlap.
static void beforeUploadBuild()
{
    GfxUploadBatch* pBatch = &gfxUploads.batches[gfxUploads.current];

    if (pBatch->transferred) {
        gfxMemoryBarrier(pBatch->cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                         VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                         VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR |
                             VK_ACCESS_2_TRANSFER_WRITE_BIT);
        pBatch->transferred = false;
        pBatch->writtenCount = 0;
    }

    pBatch->built = true;
}

void gfxCopyBufferFromHost(const GfxBuffer* pBuffer, const void* pData, VkDeviceSize size, VkDeviceSize offset)
{
    // Host coherent memory is written directly
    if (pBuffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        memcpy((char*)pBuffer->pHostMap + offset, pData, size);
        return;
    }

    if (!size) {
        return;
    }

    GfxStaging staging = allocateStaging(size, 0);
    memcpy(staging.pData, pData, size);

    VkCommandBuffer cmd = uploadCommandBuffer();
    beforeUploadCopy(pBuffer->buffer);

    VkBufferCopy region = {
        .srcOffset = staging.offset,
        .dstOffset = offset,
        .size = size,
    };
    vkCmdCopyBuffer(cmd, staging.buffer, pBuffer->buffer, 1, &region);
}

VkDeviceAddress gfxGetBufferDeviceAddress(const GfxBuffer* pBuffer)
{
    if (!(pBuffer->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
        GFX_ERROR("Buffer was not created with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT");
        return 0;
    }

    VkBufferDeviceAddressInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = pBuffer->buffer,
    };
    return vkGetBufferDeviceAddress(gfxDevice.device, &info);
}

void gfxCreatePerFrameBuffer(VkDeviceSize size, VkBufferUsageFlags usage, GfxBuffer* pBuffer)
{
    // Every copy has to be usable as the offset of a buffer descriptor
    const VkPhysicalDeviceLimits* pLimits = &gfxDevice.properties.physicalDevice.limits;
    VkDeviceSize alignment =
        GFX_MAX(GFX_MAX(pLimits->minUniformBufferOffsetAlignment, pLimits->minStorageBufferOffsetAlignment), 16);

    uint32_t frameCount = GFX_MAX(gfxSwapchain.framesInFlight, 1);
    VkDeviceSize stride = gfxAlignTo(size, alignment);

    gfxCreateBuffer(stride * frameCount, usage,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, pBuffer);

    pBuffer->frameCount = frameCount;
    pBuffer->frameSize = size;
    pBuffer->frameStride = stride;
}

VkDeviceSize gfxGetPerFrameOffset(const GfxBuffer* pBuffer)
{
    if (!pBuffer->frameCount) {
        return 0;
    }
    return (gfxSwapchain.inFlightIndex % pBuffer->frameCount) * pBuffer->frameStride;
}

void* gfxGetPerFrameData(const GfxBuffer* pBuffer)
{
    return (char*)pBuffer->pHostMap + gfxGetPerFrameOffset(pBuffer);
}

void gfxCreateImage(VkExtent3D extent, uint32_t arrayLayers, uint32_t mipLevels, VkSampleCountFlagBits samples,
                    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                    VkImageType imageType, VkMemoryPropertyFlags properties, GfxImage* pImage)
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    *pImage = (GfxImage){
        .width = extent.width,
        .height = extent.height,
        .depth = extent.depth,
        .arrayLayers = arrayLayers,
        .mipLevels = mipLevels,
        .format = format,
        .tiling = tiling,
        .imageType = imageType,
        .samples = samples,
    };

    VkImageCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = flags,
        .imageType = imageType,
        .format = format,
        .extent = extent,
        .mipLevels = mipLevels,
        .arrayLayers = arrayLayers,
        .samples = samples,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VK_CHECK(vkCreateImage(gfxDevice.device, &ci, NULL, &pImage->image));

    VkMemoryDedicatedRequirements dedicatedRequirements = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS,
    };
    VkMemoryRequirements2 requirements = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = &dedicatedRequirements,
    };
    VkImageMemoryRequirementsInfo2 requirementsInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
        .image = pImage->image,
    };
    vkGetImageMemoryRequirements2(gfxDevice.device, &requirementsInfo, &requirements);

    // Drivers often want render targets alone, which lets them compress them
    VkMemoryDedicatedAllocateInfo dedicatedInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
        .image = pImage->image,
    };
    GfxMemoryAllocation allocation = allocateMemory(
        &requirements.memoryRequirements, properties, tiling == VK_IMAGE_TILING_OPTIMAL,
        dedicatedRequirements.prefersDedicatedAllocation || dedicatedRequirements.requiresDedicatedAllocation,
        &dedicatedInfo);

    pImage->memory = allocation.memory;
    pImage->memoryOffset = allocation.offset;
    pImage->memorySize = allocation.size;

    VK_CHECK(vkBindImageMemory(gfxDevice.device, pImage->image, pImage->memory, pImage->memoryOffset));
}

void gfxDestroyImage(GfxImage* pImage)
{
    // A pending upload may still write the image
    gfxFlushUploads(true);
    vkDeviceWaitIdle(gfxDevice.device);

    vkDestroyImageView(gfxDevice.device, pImage->imageView, NULL);
    vkDestroyImage(gfxDevice.device, pImage->image, NULL);
    freeMemory(pImage->memory, pImage->memoryOffset, pImage->memorySize);
    gfxDevice.resourceGeneration++;

    GFX_RESET(pImage);
}

void gfxCreateImageView(GfxImage* pImage, VkImageAspectFlags aspectFlags, VkImageViewType viewType)
{
    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
    }

    pImage->viewType = viewType;

    VkImageViewCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = pImage->image,
        .viewType = viewType,
        .format = pImage->format,
        .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {.aspectMask = aspectFlags,
                             .baseMipLevel = 0,
                             .levelCount = pImage->mipLevels,
                             .baseArrayLayer = 0,
                             .layerCount = pImage->arrayLayers},
    };

    VK_CHECK(vkCreateImageView(gfxDevice.device, &ci, NULL, &pImage->imageView));
}

void gfxCopyImageToHost(const GfxImage* pImage, VkImageLayout layout, void* pData)
{
    uint32_t texelSize = gfxFormatSize(pImage->format);
    if (!texelSize) {
        GFX_ERROR("Only images with uncompressed formats can be copied to host memory");
        return;
    }

    if (layout == VK_IMAGE_LAYOUT_UNDEFINED) {
        GFX_ERROR("An image in VK_IMAGE_LAYOUT_UNDEFINED has no contents to copy");
        return;
    }

    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    if (pImage->format == VK_FORMAT_D16_UNORM || pImage->format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
        pImage->format == VK_FORMAT_D32_SFLOAT) {
        aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (pImage->format == VK_FORMAT_S8_UINT) {
        aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkDeviceSize size = (VkDeviceSize)pImage->width * pImage->height * pImage->depth * texelSize;

    GfxBuffer staging;
    gfxCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging);

    VkImageSubresourceRange range = {
        .aspectMask = aspect,
        .levelCount = 1,
        .layerCount = 1,
    };

    // Copies can read from the general layout, any other layout is changed for
    // the copy and back again
    VkImageLayout copyLayout =
        layout == VK_IMAGE_LAYOUT_GENERAL ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VkCommandBuffer cmd = gfxCmdBegin();

    // Wait for everything submitted before, which may write the image
    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, layout, copyLayout, pImage->image,
                    &range);

    VkBufferImageCopy region = {
        .imageSubresource = {.aspectMask = aspect, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
        .imageExtent = {.width = pImage->width, .height = pImage->height, .depth = pImage->depth},
    };
    vkCmdCopyImageToBuffer(cmd, pImage->image, copyLayout, staging.buffer, 1, &region);

    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                    VK_ACCESS_2_NONE, copyLayout, layout, pImage->image, &range);

    gfxCmdEnd(cmd);

    memcpy(pData, staging.pHostMap, size);

    gfxDestroyBuffer(&staging);
}

#ifdef GFX_USE_STB_IMAGE_WRITE
bool gfxSaveImagePNG(const GfxImage* pImage, VkImageLayout layout, const char* pPath)
{
    bool bgra;
    switch (pImage->format) {
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SRGB:
        bgra = false;
        break;
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SRGB:
        bgra = true;
        break;
    default:
        GFX_WARNING("Only images with 8 bit RGBA or BGRA formats can be saved as PNG");
        return false;
    }

    size_t pixelCount = (size_t)pImage->width * pImage->height;
    uint8_t* pPixels = GFX_MALLOC(pixelCount * 4);
    gfxCopyImageToHost(pImage, layout, pPixels);

    if (bgra) {
        for (size_t i = 0; i < pixelCount; i++) {
            uint8_t blue = pPixels[4 * i + 0];
            pPixels[4 * i + 0] = pPixels[4 * i + 2];
            pPixels[4 * i + 2] = blue;
        }
    }

    bool saved = stbi_write_png(pPath, (int)pImage->width, (int)pImage->height, 4, pPixels, (int)pImage->width * 4);

    GFX_FREE(pPixels);

    if (saved) {
        GFX_INFO("Saved image to %s", pPath);
    } else {
        GFX_WARNING("Failed to save image to %s", pPath);
    }
    return saved;
}
#endif

static void generateMipmaps(VkCommandBuffer cmd, GfxTexture* pTexture)
{
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(gfxDevice.physicalDevice, pTexture->image.format, &props);

    if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        GFX_ERROR("Texture image format does not support linear blitting");
    }

    VkImageSubresourceRange subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = pTexture->image.arrayLayers,
    };

    int32_t mipWidth = pTexture->image.width;
    int32_t mipHeight = pTexture->image.height;
    int32_t mipDepth = pTexture->image.depth;

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
                               .layerCount = pTexture->image.arrayLayers},
            .srcOffsets = {{0, 0, 0}, {mipWidth, mipHeight, mipDepth}},
            .dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                               .mipLevel = i,
                               .baseArrayLayer = 0,
                               .layerCount = pTexture->image.arrayLayers},
            .dstOffsets = {{0, 0, 0},
                           {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1,
                            mipDepth > 1 ? mipDepth / 2 : 1}},
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
        if (mipDepth > 1) {
            mipDepth /= 2;
        }
    }

    subresourceRange.baseMipLevel = pTexture->image.mipLevels - 1;

    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    pTexture->image.image, &subresourceRange);
}

static void createTexture(GfxTexture* pTexture, enum GfxTextureType type, VkFormat format, const void* pData,
                          VkExtent3D extent, bool mipmaps)
{
    VkImageType imageType;
    VkImageViewType viewType;
    VkImageCreateFlags flags = 0;
    uint32_t arrayLayers = 1;

    switch (type) {
    case GFX_TEXTURE_1D:
        imageType = VK_IMAGE_TYPE_1D;
        viewType = VK_IMAGE_VIEW_TYPE_1D;
        break;
    case GFX_TEXTURE_2D:
        imageType = VK_IMAGE_TYPE_2D;
        viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case GFX_TEXTURE_3D:
        imageType = VK_IMAGE_TYPE_3D;
        viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case GFX_TEXTURE_CUBE_MAP:
        // A cube map is six 2D layers, with pData holding the faces in the
        // order +X, -X, +Y, -Y, +Z, -Z
        imageType = VK_IMAGE_TYPE_2D;
        viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        arrayLayers = 6;
        break;
    default:
        GFX_ERROR("Unknown texture type");
        return;
    }

    if (imageType != VK_IMAGE_TYPE_3D && extent.depth != 1) {
        GFX_ERROR("Only 3D textures can have a depth other than 1");
    }
    if (imageType == VK_IMAGE_TYPE_1D && extent.height != 1) {
        GFX_ERROR("1D textures must have a height of 1");
    }

    uint32_t mipLevels = 1;
    if (mipmaps) {
        uint32_t largest = GFX_MAX(extent.width, extent.height);
        if (imageType == VK_IMAGE_TYPE_3D) {
            largest = GFX_MAX(largest, extent.depth);
        }
        mipLevels = (uint32_t)floor(log2(largest)) + 1;
    }

    gfxCreateImage(extent, arrayLayers, mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                   flags, imageType, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pTexture->image);

    if (pData) {
        uint32_t texelSize = gfxFormatSize(format);
        if (!texelSize) {
            GFX_ERROR("Texture data can only be uploaded for uncompressed formats");
            return;
        }

        // Widen before multiplying, so large textures do not overflow
        VkDeviceSize size = (VkDeviceSize)extent.width * extent.height * extent.depth * arrayLayers * texelSize;

        // The copy, the mipmaps and the final transition are recorded with
        // other uploads
        GfxStaging staging = allocateStaging(size, texelSize);
        memcpy(staging.pData, pData, size);

        VkCommandBuffer cmd = uploadCommandBuffer();

        gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                        VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        pTexture->image.image, NULL);

        VkBufferImageCopy region = {
            .bufferOffset = staging.offset,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .mipLevel = 0,
                                 .baseArrayLayer = 0,
                                 .layerCount = arrayLayers},
            .imageOffset = {0, 0, 0},
            .imageExtent = extent,
        };

        vkCmdCopyBufferToImage(cmd, staging.buffer, pTexture->image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &region);

        // The barrier at the end of the batch makes the image visible to every
        // stage of later work
        if (mipmaps) {
            generateMipmaps(cmd, pTexture);
        } else {
            gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            pTexture->image.image, NULL);
        }
    }

    gfxCreateImageView(&pTexture->image, VK_IMAGE_ASPECT_COLOR_BIT, viewType);

    pTexture->imageInfo = (VkDescriptorImageInfo){
        .sampler = NULL,
        .imageView = pTexture->image.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}

static void createSampler(GfxTexture* pTexture)
{
    if (pTexture->sampler) {
        /* Delete old sampler first */
        vkDeviceWaitIdle(gfxDevice.device);
        vkDestroySampler(gfxDevice.device, pTexture->sampler, NULL);
        gfxDevice.resourceGeneration++;
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
        .anisotropyEnable = gfxDevice.samplerAnisotropy,
        .maxAnisotropy =
            gfxDevice.samplerAnisotropy ? gfxDevice.properties.physicalDevice.limits.maxSamplerAnisotropy : 1.0f,
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

void gfxCreateTexture(enum GfxTextureType type, VkFormat format, const void* pData, VkExtent3D extent,
                      bool generateMipmaps, GfxTexture* pTexture)
{
    GFX_RESET(pTexture);

    createTexture(pTexture, type, format, pData, extent, generateMipmaps);
    createSampler(pTexture);
}

#ifdef GFX_USE_STB_IMAGE
void gfxCreateTextureFromFile(enum GfxTextureType type, VkFormat format, const char* pPath, bool generateMipmaps,
                              GfxTexture* pTexture)
{
    GFX_RESET(pTexture);

    if (type != GFX_TEXTURE_2D) {
        GFX_ERROR("Only GFX_TEXTURE_2D can be loaded from a single image file");
        return;
    }

    GFX_INFO("Loading texture from %s", pPath);

    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;
    void* pData = NULL;

    // The file is converted to four channels of what the format holds
    switch (format) {
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SRGB:
        pData = stbi_load(pPath, &width, &height, &channels, STBI_rgb_alpha);
        break;
    case VK_FORMAT_R16G16B16A16_UNORM:
        pData = stbi_load_16(pPath, &width, &height, &channels, STBI_rgb_alpha);
        break;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        pData = stbi_loadf(pPath, &width, &height, &channels, STBI_rgb_alpha);
        break;
    default:
        GFX_ERROR("Textures can only be loaded from file as VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB, "
                  "VK_FORMAT_R16G16B16A16_UNORM or VK_FORMAT_R32G32B32A32_SFLOAT");
        return;
    }

    if (!pData) {
        GFX_ERROR("Failed to load texture %s: %s", pPath, stbi_failure_reason());
        return;
    }

    VkExtent3D extent = {.width = (uint32_t)width, .height = (uint32_t)height, .depth = 1};
    createTexture(pTexture, type, format, pData, extent, generateMipmaps);
    gfxSetImageName(&pTexture->image, pPath);

    stbi_image_free(pData);

    createSampler(pTexture);
}
#endif

void gfxCreateSampler(VkFilter filter, VkSamplerAddressMode addressMode, bool compare, VkSampler* pSampler)
{
    VkSamplerCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = filter,
        .minFilter = filter,
        .mipmapMode = filter == VK_FILTER_NEAREST ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = addressMode,
        .addressModeV = addressMode,
        .addressModeW = addressMode,
        .compareEnable = compare,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
    };

    // Lit where the reference is at least as near as what the shadow map
    // holds, and the border is at the far depth so that it is lit too
    if (compare) {
#ifdef GFX_REVERSE_Z
        ci.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
        ci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
#else
        ci.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
#endif
    }

    VK_CHECK(vkCreateSampler(gfxDevice.device, &ci, NULL, pSampler));
}

void gfxDestroySampler(VkSampler sampler)
{
    vkDeviceWaitIdle(gfxDevice.device);
    vkDestroySampler(gfxDevice.device, sampler, NULL);
    gfxDevice.resourceGeneration++;
}

void gfxDestroyTexture(GfxTexture* pTexture)
{
    vkDeviceWaitIdle(gfxDevice.device);
    vkDestroySampler(gfxDevice.device, pTexture->sampler, NULL);
    gfxDestroyImage(&pTexture->image); // Also advances the resource generation

    GFX_RESET(pTexture);
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

static void createAttachmentSet(GfxAttachment* pAttachmentSet, uint32_t colorAttachmentCount,
                                GfxImage* pColorAttachments, GfxImage* pDepthAttachment, GfxImage* pResolveAttachment)
{
    // A depth-only set has no color attachments, but the arrays are still
    // allocated so that they can be freed like any other
    *pAttachmentSet = (GfxAttachment){
        .pRenderingAttachmentInfos = GFX_MALLOC(GFX_MAX(colorAttachmentCount, 1) * sizeof(VkRenderingAttachmentInfo)),
        .pFormats = GFX_MALLOC(GFX_MAX(colorAttachmentCount, 1) * sizeof(VkFormat)),
        .pColorAttachments = pColorAttachments,
        .colorAttachmentCount = colorAttachmentCount,
        .pDepthAttachment = pDepthAttachment,
        .pResolveAttachment = pResolveAttachment,
    };

    for (uint32_t i = 0; i < colorAttachmentCount; i++) {
        pAttachmentSet->pRenderingAttachmentInfos[i] = (VkRenderingAttachmentInfo){
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = pAttachmentSet->pColorAttachments[i].imageView,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = pAttachmentSet->pResolveAttachment ? VK_RESOLVE_MODE_AVERAGE_BIT : VK_RESOLVE_MODE_NONE,
            .resolveImageView =
                pAttachmentSet->pResolveAttachment ? pAttachmentSet->pResolveAttachment->imageView : NULL,
            .resolveImageLayout = pAttachmentSet->pResolveAttachment ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                                                     : VK_IMAGE_LAYOUT_UNDEFINED,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };
        pAttachmentSet->pFormats[i] = pAttachmentSet->pColorAttachments[i].format;
    }

    pAttachmentSet->pipelineRenderingCreateInfo = (VkPipelineRenderingCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = colorAttachmentCount,
        .pColorAttachmentFormats = pAttachmentSet->pFormats,
        .depthAttachmentFormat = pDepthAttachment ? pDepthAttachment->format : VK_FORMAT_UNDEFINED,
    };
}

void gfxCreateAttachment(uint32_t colorAttachmentCount, GfxImage* pColorAttachments, GfxImage* pDepthAttachment,
                         GfxImage* pResolveAttachment, GfxAttachment* pAttachmentSet)
{
    createAttachmentSet(pAttachmentSet, colorAttachmentCount, pColorAttachments, pDepthAttachment, pResolveAttachment);
}

void gfxDestroyAttachment(GfxAttachment* pAttachmentSet)
{
    GFX_FREE(pAttachmentSet->pRenderingAttachmentInfos);
    GFX_FREE(pAttachmentSet->pFormats);

    GFX_RESET(pAttachmentSet);
}

void gfxRecreateAttachment(GfxAttachment* pAttachmentSet, uint32_t colorAttachmentCount, GfxImage* pColorAttachments,
                           GfxImage* pDepthAttachment, GfxImage* pResolveAttachment)
{
    GFX_FREE(pAttachmentSet->pRenderingAttachmentInfos);
    GFX_FREE(pAttachmentSet->pFormats);
    createAttachmentSet(pAttachmentSet, colorAttachmentCount, pColorAttachments, pDepthAttachment, pResolveAttachment);
}

static bool hasStencil(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT ||
           format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

static bool isDepthFormat(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_X8_D24_UNORM_PACK32 || format == VK_FORMAT_D32_SFLOAT ||
           hasStencil(format);
}

// Layout a depth attachment is rendered in, which has to cover stencil too if
// the format has it
static VkImageLayout depthAttachmentLayout(VkFormat format)
{
    return hasStencil(format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                              : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
}

// The image that decides the size and sample count of an attachment set: the
// first color attachment, or the depth attachment of a depth-only set
static const GfxImage* attachmentSetImage(const GfxAttachment* pAttachmentSet)
{
    if (pAttachmentSet->colorAttachmentCount) {
        return &pAttachmentSet->pColorAttachments[0];
    }
    return pAttachmentSet->pDepthAttachment;
}

// Begin rendering with the clear values and load operations already in the
// attachment set
static void beginRendering(VkCommandBuffer cmd, const GfxAttachment* pAttachmentSet)
{
    VkRenderingAttachmentInfo depthAttachmentInfo;
    if (pAttachmentSet->pDepthAttachment) {
        depthAttachmentInfo = (VkRenderingAttachmentInfo){
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = pAttachmentSet->pDepthAttachment->imageView,
            .imageLayout = depthAttachmentLayout(pAttachmentSet->pDepthAttachment->format),
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue.depthStencil = {.depth = GFX_DEPTH_CLEAR, .stencil = 0},
        };
    }

    const GfxImage* pImage = attachmentSetImage(pAttachmentSet);
    if (!pImage) {
        GFX_ERROR("An attachment set needs a color or a depth attachment to render to");
        return;
    }

    VkExtent2D extent = {
        .width = pImage->width,
        .height = pImage->height,
    };

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {.offset = {0, 0}, .extent = extent},
        .layerCount = 1,
        .colorAttachmentCount = pAttachmentSet->colorAttachmentCount,
        .pColorAttachments = pAttachmentSet->pRenderingAttachmentInfos,
        .pDepthAttachment = pAttachmentSet->pDepthAttachment ? &depthAttachmentInfo : NULL,
        .pStencilAttachment = NULL,
    };

    vkCmdBeginRendering(cmd, &renderingInfo);
}

void gfxCmdBeginRendering(VkCommandBuffer cmd, const GfxAttachment* pAttachmentSet, VkClearValue clearValue,
                          VkAttachmentLoadOp loadOp)
{
    for (uint32_t i = 0; i < pAttachmentSet->colorAttachmentCount; i++) {
        pAttachmentSet->pRenderingAttachmentInfos[i].clearValue = clearValue;
        pAttachmentSet->pRenderingAttachmentInfos[i].loadOp = loadOp;
    }

    beginRendering(cmd, pAttachmentSet);
}

void gfxCmdEndRendering(VkCommandBuffer cmd, const GfxAttachment* pAttachmentSet)
{
    GFX_UNUSED(pAttachmentSet);
    vkCmdEndRendering(cmd);
}

// Report a failure as an error, or as a warning when the caller can recover
// from it, as when a shader is reloaded
#define GFX_FAIL(fatal, fmt, ...)                                                                                      \
    do {                                                                                                               \
        if (fatal) {                                                                                                   \
            GFX_ERROR(fmt, ##__VA_ARGS__);                                                                             \
        } else {                                                                                                       \
            GFX_WARNING(fmt, ##__VA_ARGS__);                                                                           \
        }                                                                                                              \
    } while (0)

// Stages that are built into a ray tracing pipeline rather than shader objects
#define GFX_RAY_TRACING_STAGES                                                                                         \
    (VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |          \
     VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)

// How many generations of every descriptor set a descriptor pool holds
#define GFX_DESCRIPTOR_SETS_PER_POOL 8

static void* allocZeroed(size_t count, size_t size)
{
    void* p = GFX_MALLOC(count * size);
    memset(p, 0, count * size);
    return p;
}

static char* copyString(const char* pString, size_t maxLength)
{
    size_t length = 0;
    while (length < maxLength && pString[length]) {
        length++;
    }
    char* pCopy = GFX_MALLOC(length + 1);
    memcpy(pCopy, pString, length);
    pCopy[length] = 0;
    return pCopy;
}

static bool endsWith(const char* pString, size_t length, const char* pSuffix)
{
    size_t suffixLength = strlen(pSuffix);
    return length >= suffixLength && !memcmp(pString + length - suffixLength, pSuffix, suffixLength);
}

// Read a whole file into memory. The contents are null terminated, which the
// size does not include. Returns NULL if the file cannot be read.
static char* readFile(const char* pPath, size_t* pSize)
{
    FILE* file = fopen(pPath, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size < 0) {
        fclose(file);
        return NULL;
    }

    char* pData = GFX_MALLOC((size_t)size + 1);
    size_t read = fread(pData, 1, (size_t)size, file);
    fclose(file);

    if (read != (size_t)size) {
        GFX_FREE(pData);
        return NULL;
    }

    pData[size] = 0;
    if (pSize) {
        *pSize = (size_t)size;
    }
    return pData;
}


// SPIR-V reflection //

// The subset of the SPIR-V specification that the reflection needs
enum {
    GFX_SPV_MAGIC = 0x07230203,

    GFX_SPV_OP_NAME = 5,
    GFX_SPV_OP_ENTRY_POINT = 15,
    GFX_SPV_OP_EXECUTION_MODE = 16,
    GFX_SPV_OP_TYPE_BOOL = 20,
    GFX_SPV_OP_TYPE_INT = 21,
    GFX_SPV_OP_TYPE_FLOAT = 22,
    GFX_SPV_OP_TYPE_VECTOR = 23,
    GFX_SPV_OP_TYPE_MATRIX = 24,
    GFX_SPV_OP_TYPE_IMAGE = 25,
    GFX_SPV_OP_TYPE_SAMPLER = 26,
    GFX_SPV_OP_TYPE_SAMPLED_IMAGE = 27,
    GFX_SPV_OP_TYPE_ARRAY = 28,
    GFX_SPV_OP_TYPE_RUNTIME_ARRAY = 29,
    GFX_SPV_OP_TYPE_STRUCT = 30,
    GFX_SPV_OP_TYPE_POINTER = 32,
    GFX_SPV_OP_CONSTANT = 43,
    GFX_SPV_OP_SPEC_CONSTANT = 50,
    GFX_SPV_OP_VARIABLE = 59,
    GFX_SPV_OP_DECORATE = 71,
    GFX_SPV_OP_MEMBER_DECORATE = 72,
    GFX_SPV_OP_EXECUTION_MODE_ID = 331,
    GFX_SPV_OP_TYPE_ACCELERATION_STRUCTURE = 5341,

    GFX_SPV_DECORATION_SPEC_ID = 1,
    GFX_SPV_DECORATION_BLOCK = 2,
    GFX_SPV_DECORATION_BUFFER_BLOCK = 3,
    GFX_SPV_DECORATION_ARRAY_STRIDE = 6,
    GFX_SPV_DECORATION_MATRIX_STRIDE = 7,
    GFX_SPV_DECORATION_BINDING = 33,
    GFX_SPV_DECORATION_DESCRIPTOR_SET = 34,
    GFX_SPV_DECORATION_OFFSET = 35,

    GFX_SPV_STORAGE_UNIFORM_CONSTANT = 0,
    GFX_SPV_STORAGE_UNIFORM = 2,
    GFX_SPV_STORAGE_PUSH_CONSTANT = 9,
    GFX_SPV_STORAGE_STORAGE_BUFFER = 12,

    GFX_SPV_DIM_BUFFER = 5,
    GFX_SPV_DIM_SUBPASS_DATA = 6,

    GFX_SPV_EXECUTION_MODE_LOCAL_SIZE = 17,
    GFX_SPV_EXECUTION_MODE_LOCAL_SIZE_ID = 38,
};

// Flags for decorations found on an id
enum {
    GFX_SPV_HAS_SET = 1 << 0,
    GFX_SPV_HAS_BINDING = 1 << 1,
    GFX_SPV_HAS_SPEC_ID = 1 << 2,
    GFX_SPV_IS_BLOCK = 1 << 3,
    GFX_SPV_IS_BUFFER_BLOCK = 1 << 4,
};

// What the reflection knows about one id of a SPIR-V module
typedef struct GfxSpvId {
    const uint32_t* pInstruction; // Instruction defining the id, for types, constants and variables
    const char* pName;            // Points into the module, not necessarily null terminated
    uint32_t nameMaxLength;
    uint32_t set;
    uint32_t binding;
    uint32_t specId;
    uint32_t arrayStride;
    uint32_t flags;
} GfxSpvId;

typedef struct GfxSpvModule {
    const uint32_t* pCode;
    uint32_t wordCount;
    uint32_t idBound;
    GfxSpvId* pIds;
    const VkSpecializationInfo* pSpecializationInfo;
} GfxSpvModule;

static const struct GfxSpvExecutionModel {
    uint32_t model;
    VkShaderStageFlagBits stage;
} gfxSpvExecutionModels[] = {
    {0, VK_SHADER_STAGE_VERTEX_BIT},
    {1, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
    {2, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
    {3, VK_SHADER_STAGE_GEOMETRY_BIT},
    {4, VK_SHADER_STAGE_FRAGMENT_BIT},
    {5, VK_SHADER_STAGE_COMPUTE_BIT},
    {5313, VK_SHADER_STAGE_RAYGEN_BIT_KHR},
    {5314, VK_SHADER_STAGE_INTERSECTION_BIT_KHR},
    {5315, VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
    {5316, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
    {5317, VK_SHADER_STAGE_MISS_BIT_KHR},
    {5318, VK_SHADER_STAGE_CALLABLE_BIT_KHR},
    {5364, VK_SHADER_STAGE_TASK_BIT_EXT},
    {5365, VK_SHADER_STAGE_MESH_BIT_EXT},
};

static uint32_t spvOpcode(const uint32_t* pInstruction)
{
    return pInstruction[0] & 0xffff;
}

static uint32_t spvWordCount(const uint32_t* pInstruction)
{
    return pInstruction[0] >> 16;
}

static bool spvIsValid(const uint32_t* pCode, size_t codeSize)
{
    return pCode && codeSize >= 5 * sizeof(uint32_t) && codeSize % sizeof(uint32_t) == 0 && pCode[0] == GFX_SPV_MAGIC;
}

// Index the ids of a module. Only the instructions the reflection needs are
// looked at, everything else is skipped.
static bool spvParse(GfxSpvModule* pModule, const uint32_t* pCode, size_t codeSize,
                     const VkSpecializationInfo* pSpecializationInfo)
{
    *pModule = (GfxSpvModule){0};

    if (!spvIsValid(pCode, codeSize)) {
        return false;
    }

    pModule->pCode = pCode;
    pModule->wordCount = (uint32_t)(codeSize / sizeof(uint32_t));
    pModule->idBound = pCode[3];
    pModule->pIds = allocZeroed(pModule->idBound, sizeof *pModule->pIds);
    pModule->pSpecializationInfo = pSpecializationInfo;

    for (uint32_t i = 5; i < pModule->wordCount;) {
        const uint32_t* in = &pCode[i];
        uint32_t n = spvWordCount(in);
        if (n == 0 || i + n > pModule->wordCount) {
            GFX_FREE(pModule->pIds);
            pModule->pIds = NULL;
            return false;
        }

        uint32_t resultId = 0;
        switch (spvOpcode(in)) {
        case GFX_SPV_OP_NAME:
            if (n >= 3 && in[1] < pModule->idBound) {
                pModule->pIds[in[1]].pName = (const char*)&in[2];
                pModule->pIds[in[1]].nameMaxLength = (n - 2) * sizeof(uint32_t);
            }
            break;

        case GFX_SPV_OP_DECORATE:
            if (n >= 3 && in[1] < pModule->idBound) {
                GfxSpvId* pId = &pModule->pIds[in[1]];
                uint32_t value = n >= 4 ? in[3] : 0;
                switch (in[2]) {
                case GFX_SPV_DECORATION_DESCRIPTOR_SET:
                    pId->set = value;
                    pId->flags |= GFX_SPV_HAS_SET;
                    break;
                case GFX_SPV_DECORATION_BINDING:
                    pId->binding = value;
                    pId->flags |= GFX_SPV_HAS_BINDING;
                    break;
                case GFX_SPV_DECORATION_SPEC_ID:
                    pId->specId = value;
                    pId->flags |= GFX_SPV_HAS_SPEC_ID;
                    break;
                case GFX_SPV_DECORATION_ARRAY_STRIDE:
                    pId->arrayStride = value;
                    break;
                case GFX_SPV_DECORATION_BLOCK:
                    pId->flags |= GFX_SPV_IS_BLOCK;
                    break;
                case GFX_SPV_DECORATION_BUFFER_BLOCK:
                    pId->flags |= GFX_SPV_IS_BUFFER_BLOCK;
                    break;
                }
            }
            break;

        // Types have their result id first
        case GFX_SPV_OP_TYPE_BOOL:
        case GFX_SPV_OP_TYPE_INT:
        case GFX_SPV_OP_TYPE_FLOAT:
        case GFX_SPV_OP_TYPE_VECTOR:
        case GFX_SPV_OP_TYPE_MATRIX:
        case GFX_SPV_OP_TYPE_IMAGE:
        case GFX_SPV_OP_TYPE_SAMPLER:
        case GFX_SPV_OP_TYPE_SAMPLED_IMAGE:
        case GFX_SPV_OP_TYPE_ARRAY:
        case GFX_SPV_OP_TYPE_RUNTIME_ARRAY:
        case GFX_SPV_OP_TYPE_STRUCT:
        case GFX_SPV_OP_TYPE_POINTER:
        case GFX_SPV_OP_TYPE_ACCELERATION_STRUCTURE:
            resultId = n >= 2 ? in[1] : 0;
            break;

        // Constants and variables have their result type first
        case GFX_SPV_OP_CONSTANT:
        case GFX_SPV_OP_SPEC_CONSTANT:
        case GFX_SPV_OP_VARIABLE:
            resultId = n >= 3 ? in[2] : 0;
            break;
        }

        if (resultId && resultId < pModule->idBound) {
            pModule->pIds[resultId].pInstruction = in;
        }

        i += n;
    }

    return true;
}

static const uint32_t* spvInstruction(const GfxSpvModule* pModule, uint32_t id)
{
    return id < pModule->idBound ? pModule->pIds[id].pInstruction : NULL;
}

// Get the name of an id as a new string, empty if it has none
static char* spvName(const GfxSpvModule* pModule, uint32_t id)
{
    const GfxSpvId* pId = &pModule->pIds[id];
    return pId->pName ? copyString(pId->pName, pId->nameMaxLength) : copyString("", 0);
}

// Get the value of an integer constant. Specialization constants take their
// value from the specialization info if it sets them.
static bool spvConstant(const GfxSpvModule* pModule, uint32_t id, uint32_t* pValue)
{
    const uint32_t* in = spvInstruction(pModule, id);
    if (!in || spvWordCount(in) < 4 ||
        (spvOpcode(in) != GFX_SPV_OP_CONSTANT && spvOpcode(in) != GFX_SPV_OP_SPEC_CONSTANT)) {
        return false;
    }

    *pValue = in[3];

    const GfxSpvId* pId = &pModule->pIds[id];
    const VkSpecializationInfo* pInfo = pModule->pSpecializationInfo;
    if (spvOpcode(in) == GFX_SPV_OP_SPEC_CONSTANT && (pId->flags & GFX_SPV_HAS_SPEC_ID) && pInfo) {
        for (uint32_t i = 0; i < pInfo->mapEntryCount; i++) {
            const VkSpecializationMapEntry* pEntry = &pInfo->pMapEntries[i];
            if (pEntry->constantID == pId->specId) {
                uint32_t value = 0;
                memcpy(&value, (const char*)pInfo->pData + pEntry->offset, GFX_MIN(pEntry->size, sizeof value));
                *pValue = value;
                break;
            }
        }
    }

    return true;
}

static bool spvMemberDecoration(const GfxSpvModule* pModule, uint32_t structId, uint32_t member, uint32_t decoration,
                                uint32_t* pValue)
{
    // Already validated by spvParse()
    for (uint32_t i = 5; i < pModule->wordCount; i += spvWordCount(&pModule->pCode[i])) {
        const uint32_t* in = &pModule->pCode[i];
        if (spvOpcode(in) == GFX_SPV_OP_MEMBER_DECORATE && spvWordCount(in) >= 5 && in[1] == structId &&
            in[2] == member && in[3] == decoration) {
            *pValue = in[4];
            return true;
        }
    }
    return false;
}

// Size in bytes of a type in an explicitly laid out block, such as a push
// constant block. Structs end at their last member, without trailing padding.
static uint32_t spvTypeSize(const GfxSpvModule* pModule, uint32_t typeId, uint32_t matrixStride, uint32_t depth)
{
    const uint32_t* in = spvInstruction(pModule, typeId);
    if (!in || depth > 32) {
        return 0;
    }

    switch (spvOpcode(in)) {
    case GFX_SPV_OP_TYPE_BOOL:
        return 4;
    case GFX_SPV_OP_TYPE_INT:
    case GFX_SPV_OP_TYPE_FLOAT:
        return in[2] / 8;
    case GFX_SPV_OP_TYPE_VECTOR:
        return in[3] * spvTypeSize(pModule, in[2], 0, depth + 1);
    case GFX_SPV_OP_TYPE_MATRIX:
        return in[3] * (matrixStride ? matrixStride : spvTypeSize(pModule, in[2], 0, depth + 1));
    case GFX_SPV_OP_TYPE_ARRAY: {
        uint32_t length = 0;
        spvConstant(pModule, in[3], &length);
        uint32_t stride = pModule->pIds[typeId].arrayStride;
        return length * (stride ? stride : spvTypeSize(pModule, in[2], matrixStride, depth + 1));
    }
    case GFX_SPV_OP_TYPE_STRUCT: {
        uint32_t size = 0;
        for (uint32_t member = 0; member + 2 < spvWordCount(in); member++) {
            uint32_t offset = 0;
            uint32_t memberMatrixStride = 0;
            spvMemberDecoration(pModule, typeId, member, GFX_SPV_DECORATION_OFFSET, &offset);
            spvMemberDecoration(pModule, typeId, member, GFX_SPV_DECORATION_MATRIX_STRIDE, &memberMatrixStride);
            size = GFX_MAX(size, offset + spvTypeSize(pModule, in[2 + member], memberMatrixStride, depth + 1));
        }
        return size;
    }
    case GFX_SPV_OP_TYPE_POINTER:
        // Physical storage buffer pointers, from buffer device addresses
        return 8;
    }

    return 0;
}

static bool spvDescriptorType(const GfxSpvModule* pModule, uint32_t typeId, uint32_t storageClass,
                              VkDescriptorType* pType)
{
    const uint32_t* in = spvInstruction(pModule, typeId);
    if (!in) {
        return false;
    }

    switch (spvOpcode(in)) {
    case GFX_SPV_OP_TYPE_STRUCT:
        if (storageClass == GFX_SPV_STORAGE_STORAGE_BUFFER ||
            (storageClass == GFX_SPV_STORAGE_UNIFORM && (pModule->pIds[typeId].flags & GFX_SPV_IS_BUFFER_BLOCK))) {
            *pType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            return true;
        }
        if (storageClass == GFX_SPV_STORAGE_UNIFORM) {
            *pType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            return true;
        }
        return false;

    case GFX_SPV_OP_TYPE_SAMPLED_IMAGE: {
        const uint32_t* pImage = spvInstruction(pModule, in[2]);
        bool isBuffer = pImage && pImage[3] == GFX_SPV_DIM_BUFFER;
        *pType = isBuffer ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        return true;
    }

    case GFX_SPV_OP_TYPE_IMAGE: {
        uint32_t dim = in[3];
        bool isStorage = in[7] == 2;
        if (dim == GFX_SPV_DIM_SUBPASS_DATA) {
            *pType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        } else if (dim == GFX_SPV_DIM_BUFFER) {
            *pType = isStorage ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        } else {
            *pType = isStorage ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        }
        return true;
    }

    case GFX_SPV_OP_TYPE_SAMPLER:
        *pType = VK_DESCRIPTOR_TYPE_SAMPLER;
        return true;

    case GFX_SPV_OP_TYPE_ACCELERATION_STRUCTURE:
        *pType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        return true;
    }

    return false;
}

// Stage of the first entry point in a module, 0 if it has none
static VkShaderStageFlagBits spvEntryPointStage(const uint32_t* pCode, size_t codeSize)
{
    uint32_t wordCount = (uint32_t)(codeSize / sizeof(uint32_t));
    for (uint32_t i = 5; i < wordCount && spvWordCount(&pCode[i]); i += spvWordCount(&pCode[i])) {
        if (spvOpcode(&pCode[i]) == GFX_SPV_OP_ENTRY_POINT && i + 1 < wordCount) {
            for (uint32_t j = 0; j < GFX_ARRAY_LEN(gfxSpvExecutionModels); j++) {
                if (gfxSpvExecutionModels[j].model == pCode[i + 1]) {
                    return gfxSpvExecutionModels[j].stage;
                }
            }
            return 0;
        }
    }
    return 0;
}


// Shaders //

static const char* descriptorTypeName(VkDescriptorType type)
{
    switch (type) {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
        return "sampler";
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return "combined image sampler";
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        return "sampled image";
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return "storage image";
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        return "uniform texel buffer";
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        return "storage texel buffer";
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return "uniform buffer";
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return "storage buffer";
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        return "input attachment";
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        return "acceleration structure";
    default:
        return "descriptor";
    }
}

static bool isBufferDescriptor(VkDescriptorType type)
{
    return type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
}

static bool isImageDescriptor(VkDescriptorType type)
{
    return type == VK_DESCRIPTOR_TYPE_SAMPLER || type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
           type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
           type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
}

// Name used for a binding in messages
static const char* bindingName(const GfxShaderBinding* pBinding)
{
    if (pBinding->pName[0]) {
        return pBinding->pName;
    }
    return pBinding->pBlockName ? pBinding->pBlockName : "(unnamed)";
}

// Name used for a shader in messages
static const char* shaderName(const GfxShader* pShader)
{
    return pShader->stageCount && pShader->pStages[0].pPath ? pShader->pStages[0].pPath : "(SPIR-V)";
}

static const struct GfxStageExtension {
    const char* pExtension;
    VkShaderStageFlagBits stage;
} gfxStageExtensions[] = {
    {".vert", VK_SHADER_STAGE_VERTEX_BIT},
    {".tesc", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
    {".tese", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
    {".geom", VK_SHADER_STAGE_GEOMETRY_BIT},
    {".frag", VK_SHADER_STAGE_FRAGMENT_BIT},
    {".comp", VK_SHADER_STAGE_COMPUTE_BIT},
    {".task", VK_SHADER_STAGE_TASK_BIT_EXT},
    {".mesh", VK_SHADER_STAGE_MESH_BIT_EXT},
    {".rgen", VK_SHADER_STAGE_RAYGEN_BIT_KHR},
    {".rint", VK_SHADER_STAGE_INTERSECTION_BIT_KHR},
    {".rahit", VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
    {".rchit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
    {".rmiss", VK_SHADER_STAGE_MISS_BIT_KHR},
    {".rcall", VK_SHADER_STAGE_CALLABLE_BIT_KHR},
};

static VkShaderStageFlagBits stageFromPath(const char* pPath)
{
    size_t length = strlen(pPath);

    // Allow a trailing .glsl, as in shader.frag.glsl
    if (endsWith(pPath, length, ".glsl")) {
        length -= strlen(".glsl");
    }

    for (uint32_t i = 0; i < GFX_ARRAY_LEN(gfxStageExtensions); i++) {
        if (endsWith(pPath, length, gfxStageExtensions[i].pExtension)) {
            return gfxStageExtensions[i].stage;
        }
    }

    return 0;
}

static glslang_stage_t glslangStage(VkShaderStageFlagBits stage)
{
    switch (stage) {
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        return GLSLANG_STAGE_TESSCONTROL;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        return GLSLANG_STAGE_TESSEVALUATION;
    case VK_SHADER_STAGE_GEOMETRY_BIT:
        return GLSLANG_STAGE_GEOMETRY;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        return GLSLANG_STAGE_FRAGMENT;
    case VK_SHADER_STAGE_COMPUTE_BIT:
        return GLSLANG_STAGE_COMPUTE;
    case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        return GLSLANG_STAGE_RAYGEN;
    case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        return GLSLANG_STAGE_INTERSECT;
    case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        return GLSLANG_STAGE_ANYHIT;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        return GLSLANG_STAGE_CLOSESTHIT;
    case VK_SHADER_STAGE_MISS_BIT_KHR:
        return GLSLANG_STAGE_MISS;
    case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
        return GLSLANG_STAGE_CALLABLE;
    case VK_SHADER_STAGE_TASK_BIT_EXT:
        return GLSLANG_STAGE_TASK;
    case VK_SHADER_STAGE_MESH_BIT_EXT:
        return GLSLANG_STAGE_MESH;
    default:
        return GLSLANG_STAGE_VERTEX;
    }
}

// Resolve #include relative to the including file. glslang knows the top
// level file by no name, so it is passed as the context, and nested includes
// are known by the path returned here.
static glsl_include_result_t* includeFile(void* ctx, const char* pHeaderName, const char* pIncluderName,
                                          size_t includeDepth)
{
    GFX_UNUSED(includeDepth);

    const char* pIncluder = pIncluderName && pIncluderName[0] ? pIncluderName : (const char*)ctx;

    size_t directoryLength = 0;
    for (const char* c = pIncluder; *c; c++) {
        if (*c == '/' || *c == '\\') {
            directoryLength = (size_t)(c - pIncluder) + 1;
        }
    }

    size_t headerLength = strlen(pHeaderName);
    char* pPath = GFX_MALLOC(directoryLength + headerLength + 1);
    memcpy(pPath, pIncluder, directoryLength);
    memcpy(pPath + directoryLength, pHeaderName, headerLength + 1);

    size_t size = 0;
    char* pData = readFile(pPath, &size);
    if (!pData) {
        // A missing header name makes glslang report the failed include
        GFX_FREE(pPath);
        pPath = NULL;
    }

    glsl_include_result_t* pResult = GFX_MALLOC(sizeof *pResult);
    *pResult = (glsl_include_result_t){
        .header_name = pPath,
        .header_data = pData,
        .header_length = size,
    };
    return pResult;
}

static int freeIncludeResult(void* ctx, glsl_include_result_t* pResult)
{
    GFX_UNUSED(ctx);
    GFX_FREE((void*)pResult->header_name);
    GFX_FREE((void*)pResult->header_data);
    GFX_FREE(pResult);
    return 0;
}

// Enable #include in a GLSL source, without every shader having to enable the
// extension. The extension has to come after #version, so it is inserted on
// the line after it, followed by a #line directive that keeps the line numbers
// in messages matching the file. Takes ownership of the source.
static char* enableIncludes(char* pSource)
{
    // Find the line holding #version
    const char* pVersion = NULL;
    uint32_t line = 1;
    for (const char* c = pSource; *c; line++) {
        while (*c == ' ' || *c == '\t') {
            c++;
        }
        if (!strncmp(c, "#version", strlen("#version"))) {
            pVersion = c;
            break;
        }
        while (*c && *c != '\n') {
            c++;
        }
        if (*c) {
            c++;
        }
    }

    // Without #version, glslang reports the error
    const char* pLineEnd = pVersion ? strchr(pVersion, '\n') : NULL;
    if (!pLineEnd) {
        return pSource;
    }

    char insert[96];
    snprintf(insert, sizeof insert, "#extension GL_GOOGLE_include_directive : require\n#line %" PRIu32 "\n", line + 1);

    size_t headLength = (size_t)(pLineEnd + 1 - pSource);
    size_t insertLength = strlen(insert);
    size_t tailLength = strlen(pLineEnd + 1);

    char* pResult = GFX_MALLOC(headLength + insertLength + tailLength + 1);
    memcpy(pResult, pSource, headLength);
    memcpy(pResult + headLength, insert, insertLength);
    memcpy(pResult + headLength + insertLength, pLineEnd + 1, tailLength + 1);

    GFX_FREE(pSource);
    return pResult;
}

// Compile GLSL to SPIR-V. Returns NULL and reports the errors if it fails.
static uint32_t* compileGLSL(const char* pPath, VkShaderStageFlagBits stage, size_t* pCodeSize, bool fatal)
{
    char* pSource = readFile(pPath, NULL);
    if (!pSource) {
        GFX_FAIL(fatal, "Unable to open %s", pPath);
        return NULL;
    }
    pSource = enableIncludes(pSource);

    // Target the SPIR-V version that the Vulkan version guarantees
    glslang_target_client_version_t clientVersion = GLSLANG_TARGET_VULKAN_1_0;
    glslang_target_language_version_t spirvVersion = GLSLANG_TARGET_SPV_1_0;
    if (VK_API_VERSION_MAJOR(gfxInstance.apiVersion) == 1) {
        switch (VK_API_VERSION_MINOR(gfxInstance.apiVersion)) {
        case 0:
            break;
        case 1:
            clientVersion = GLSLANG_TARGET_VULKAN_1_1;
            spirvVersion = GLSLANG_TARGET_SPV_1_3;
            break;
        case 2:
            clientVersion = GLSLANG_TARGET_VULKAN_1_2;
            spirvVersion = GLSLANG_TARGET_SPV_1_5;
            break;
        case 3:
            clientVersion = GLSLANG_TARGET_VULKAN_1_3;
            spirvVersion = GLSLANG_TARGET_SPV_1_6;
            break;
        default:
            clientVersion = GLSLANG_TARGET_VULKAN_1_4;
            spirvVersion = GLSLANG_TARGET_SPV_1_6;
            break;
        }
    }

    const glslang_input_t input = {
        .language = GLSLANG_SOURCE_GLSL,
        .stage = glslangStage(stage),
        .client = GLSLANG_CLIENT_VULKAN,
        .client_version = clientVersion,
        .target_language = GLSLANG_TARGET_SPV,
        .target_language_version = spirvVersion,
        .code = pSource,
        .default_version = 100,
        .default_profile = GLSLANG_NO_PROFILE,
        .force_default_version_and_profile = false,
        .forward_compatible = false,
        .messages = GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT,
        .resource = glslang_default_resource(),
        .callbacks =
            {
                .include_system = includeFile,
                .include_local = includeFile,
                .free_include_result = freeIncludeResult,
            },
        .callbacks_ctx = (void*)pPath,
    };

    glslang_shader_t* shader = glslang_shader_create(&input);
    glslang_program_t* program = NULL;
    uint32_t* pCode = NULL;

    if (!glslang_shader_preprocess(shader, &input)) {
        GFX_FAIL(fatal, "GLSL preprocessing failed %s\n%s%s", pPath, glslang_shader_get_info_log(shader),
                 glslang_shader_get_info_debug_log(shader));
        goto done;
    }

    if (!glslang_shader_parse(shader, &input)) {
        GFX_FAIL(fatal, "GLSL parsing failed %s\n%s%s", pPath, glslang_shader_get_info_log(shader),
                 glslang_shader_get_info_debug_log(shader));
        goto done;
    }

    program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
        GFX_FAIL(fatal, "GLSL linking failed %s\n%s%s", pPath, glslang_program_get_info_log(program),
                 glslang_program_get_info_debug_log(program));
        goto done;
    }

    // The optimizer is left out so that the names used by the reflection are
    // kept. The driver optimizes the shader anyway.
    glslang_spv_options_t options = {
#ifndef NDEBUG
        .generate_debug_info = true,
#endif
        .disable_optimizer = true,
    };
    glslang_program_SPIRV_generate_with_options(program, input.stage, &options);

    const char* pMessages = glslang_program_SPIRV_get_messages(program);
    if (pMessages && pMessages[0]) {
        GFX_WARNING("%s: %s", pPath, pMessages);
    }

    *pCodeSize = glslang_program_SPIRV_get_size(program) * sizeof(uint32_t);
    pCode = GFX_MALLOC(*pCodeSize);
    glslang_program_SPIRV_get(program, pCode);

done:
    if (program) {
        glslang_program_delete(program);
    }
    glslang_shader_delete(shader);
    GFX_FREE(pSource);

    return pCode;
}

// Copy a specialization info, its map entries and data into one allocation
static VkSpecializationInfo* copySpecializationInfo(const VkSpecializationInfo* pInfo)
{
    if (!pInfo) {
        return NULL;
    }

    size_t entriesSize = pInfo->mapEntryCount * sizeof *pInfo->pMapEntries;
    VkSpecializationInfo* pCopy = GFX_MALLOC(sizeof *pCopy + entriesSize + pInfo->dataSize);
    VkSpecializationMapEntry* pEntries = (VkSpecializationMapEntry*)(pCopy + 1);
    void* pData = (char*)pEntries + entriesSize;

    if (entriesSize) {
        memcpy(pEntries, pInfo->pMapEntries, entriesSize);
    }
    if (pInfo->dataSize) {
        memcpy(pData, pInfo->pData, pInfo->dataSize);
    }

    *pCopy = (VkSpecializationInfo){
        .mapEntryCount = pInfo->mapEntryCount,
        .pMapEntries = pEntries,
        .dataSize = pInfo->dataSize,
        .pData = pData,
    };
    return pCopy;
}

// Get the SPIR-V of a stage, compiling GLSL if needed
static bool loadStage(GfxShaderStage* pStage, const GfxShaderStageDesc* pDesc, bool fatal)
{
    pStage->stage = pDesc->stage;
    pStage->pSpecializationInfo = copySpecializationInfo(pDesc->pSpecializationInfo);

    if (pDesc->pPath) {
        pStage->pPath = copyString(pDesc->pPath, SIZE_MAX);

        if (endsWith(pDesc->pPath, strlen(pDesc->pPath), ".spv")) {
            pStage->pCode = (uint32_t*)readFile(pDesc->pPath, &pStage->codeSize);
            if (!pStage->pCode) {
                GFX_FAIL(fatal, "Unable to open %s", pDesc->pPath);
                return false;
            }
        } else {
            if (!pStage->stage) {
                pStage->stage = stageFromPath(pDesc->pPath);
            }
            if (!pStage->stage) {
                GFX_FAIL(fatal, "Cannot tell the stage of %s from its extension, set GfxShaderStageDesc::stage",
                         pDesc->pPath);
                return false;
            }

            pStage->pCode = compileGLSL(pDesc->pPath, pStage->stage, &pStage->codeSize, fatal);
            if (!pStage->pCode) {
                return false;
            }
        }
    } else if (pDesc->pCode && pDesc->codeSize) {
        pStage->codeSize = pDesc->codeSize;
        pStage->pCode = GFX_MALLOC(pDesc->codeSize);
        memcpy(pStage->pCode, pDesc->pCode, pDesc->codeSize);
    } else {
        GFX_FAIL(fatal, "A shader stage needs either a path or SPIR-V code");
        return false;
    }

    const char* pName = pStage->pPath ? pStage->pPath : "(SPIR-V)";

    if (!spvIsValid(pStage->pCode, pStage->codeSize)) {
        GFX_FAIL(fatal, "%s is not valid SPIR-V", pName);
        return false;
    }

    if (!pStage->stage) {
        pStage->stage = spvEntryPointStage(pStage->pCode, pStage->codeSize);
    }
    if (!pStage->stage) {
        GFX_FAIL(fatal, "%s has no entry point to tell its stage from", pName);
        return false;
    }

    return true;
}

// Add a reflected binding to the shader, or merge it with the same binding
// from another stage. Takes ownership of the names.
static bool addBinding(GfxShader* pShader, uint32_t* pCapacity, GfxShaderBinding binding, const char* pStageName,
                       bool fatal)
{
    for (uint32_t i = 0; i < pShader->bindingCount; i++) {
        GfxShaderBinding* pExisting = &pShader->pBindings[i];
        if (pExisting->set != binding.set || pExisting->binding != binding.binding) {
            continue;
        }

        GFX_FREE(binding.pName);
        GFX_FREE(binding.pBlockName);

        if (pExisting->type != binding.type || pExisting->count != binding.count) {
            GFX_FAIL(fatal, "%s: set %" PRIu32 " binding %" PRIu32 " is declared differently in different stages",
                     pStageName, binding.set, binding.binding);
            return false;
        }

        pExisting->stages |= binding.stages;
        return true;
    }

    if (pShader->bindingCount == *pCapacity) {
        *pCapacity = *pCapacity ? *pCapacity * 2 : 8;
        pShader->pBindings = GFX_REALLOC(pShader->pBindings, *pCapacity * sizeof *pShader->pBindings);
        if (!pShader->pBindings) {
            GFX_ERROR("Unable to allocate memory");
        }
    }

    pShader->pBindings[pShader->bindingCount++] = binding;
    return true;
}

// Reflect the resources, push constants and local size of a stage
static bool reflectStage(GfxShader* pShader, const GfxShaderStage* pStage, uint32_t* pBindingCapacity, bool fatal)
{
    const char* pStageName = pStage->pPath ? pStage->pPath : "(SPIR-V)";

    GfxSpvModule module;
    if (!spvParse(&module, pStage->pCode, pStage->codeSize, pStage->pSpecializationInfo)) {
        GFX_FAIL(fatal, "%s is not valid SPIR-V", pStageName);
        return false;
    }

    bool ok = true;

    for (uint32_t id = 1; id < module.idBound && ok; id++) {
        const uint32_t* in = module.pIds[id].pInstruction;
        if (!in || spvOpcode(in) != GFX_SPV_OP_VARIABLE || spvWordCount(in) < 4) {
            continue;
        }

        uint32_t storageClass = in[3];
        const uint32_t* pPointer = spvInstruction(&module, in[1]);
        if (!pPointer || spvOpcode(pPointer) != GFX_SPV_OP_TYPE_POINTER) {
            continue;
        }
        uint32_t typeId = pPointer[3];

        if (storageClass == GFX_SPV_STORAGE_PUSH_CONSTANT) {
            uint32_t size = (uint32_t)gfxAlignTo(spvTypeSize(&module, typeId, 0, 0), 4);
            pShader->pushConstantRange.stageFlags |= pStage->stage;
            pShader->pushConstantRange.size = GFX_MAX(pShader->pushConstantRange.size, size);
            continue;
        }

        if (storageClass != GFX_SPV_STORAGE_UNIFORM_CONSTANT && storageClass != GFX_SPV_STORAGE_UNIFORM &&
            storageClass != GFX_SPV_STORAGE_STORAGE_BUFFER) {
            continue;
        }

        const GfxSpvId* pVariable = &module.pIds[id];
        if (!(pVariable->flags & GFX_SPV_HAS_BINDING)) {
            continue;
        }

        // Arrays of descriptors. Arrays of arrays are flattened.
        uint32_t count = 1;
        for (const uint32_t* t = spvInstruction(&module, typeId);
             t && (spvOpcode(t) == GFX_SPV_OP_TYPE_ARRAY || spvOpcode(t) == GFX_SPV_OP_TYPE_RUNTIME_ARRAY);
             t = spvInstruction(&module, typeId)) {
            uint32_t length = 0;
            if (spvOpcode(t) == GFX_SPV_OP_TYPE_RUNTIME_ARRAY || !spvConstant(&module, t[3], &length)) {
                GFX_FAIL(fatal,
                         "%s: binding %" PRIu32 " is an unsized array. Give it a size, or size it with a "
                         "specialization constant.",
                         pStageName, pVariable->binding);
                ok = false;
                break;
            }
            count *= length;
            typeId = t[2];
        }
        if (!ok) {
            break;
        }

        VkDescriptorType type;
        if (!spvDescriptorType(&module, typeId, storageClass, &type)) {
            GFX_FAIL(fatal, "%s: binding %" PRIu32 " has a resource type that is not supported", pStageName,
                     pVariable->binding);
            ok = false;
            break;
        }

        bool isBlock = module.pIds[typeId].flags & (GFX_SPV_IS_BLOCK | GFX_SPV_IS_BUFFER_BLOCK);

        GfxShaderBinding binding = {
            .pName = spvName(&module, id),
            .pBlockName = isBlock ? spvName(&module, typeId) : NULL,
            .set = (pVariable->flags & GFX_SPV_HAS_SET) ? pVariable->set : 0,
            .binding = pVariable->binding,
            .type = type,
            .count = count,
            .stages = pStage->stage,
        };

        ok = addBinding(pShader, pBindingCapacity, binding, pStageName, fatal);
    }

    // Workgroup size, either given as literals or as constant ids
    for (uint32_t i = 5; i < module.wordCount; i += spvWordCount(&module.pCode[i])) {
        const uint32_t* in = &module.pCode[i];
        if (spvWordCount(in) < 6) {
            continue;
        }
        if (spvOpcode(in) == GFX_SPV_OP_EXECUTION_MODE && in[2] == GFX_SPV_EXECUTION_MODE_LOCAL_SIZE) {
            memcpy(pShader->localSize, &in[3], sizeof pShader->localSize);
        } else if (spvOpcode(in) == GFX_SPV_OP_EXECUTION_MODE_ID && in[2] == GFX_SPV_EXECUTION_MODE_LOCAL_SIZE_ID) {
            for (uint32_t j = 0; j < 3; j++) {
                spvConstant(&module, in[3 + j], &pShader->localSize[j]);
            }
        }
    }

    GFX_FREE(module.pIds);
    return ok;
}

static int compareBindings(const void* pA, const void* pB)
{
    const GfxShaderBinding* a = pA;
    const GfxShaderBinding* b = pB;
    if (a->set != b->set) {
        return a->set < b->set ? -1 : 1;
    }
    if (a->binding != b->binding) {
        return a->binding < b->binding ? -1 : 1;
    }
    return 0;
}

// Create descriptor set layouts, the pipeline layout and the descriptor info
// storage from the reflected bindings
static bool createLayouts(GfxShader* pShader, bool fatal)
{
    if (pShader->bindingCount) {
        qsort(pShader->pBindings, pShader->bindingCount, sizeof *pShader->pBindings, compareBindings);
        pShader->setCount = pShader->pBindings[pShader->bindingCount - 1].set + 1;
    }

    uint32_t maxSets = gfxDevice.properties.physicalDevice.limits.maxBoundDescriptorSets;
    if (pShader->setCount > maxSets) {
        GFX_FAIL(fatal, "%s uses %" PRIu32 " descriptor sets, but the device only supports %" PRIu32,
                 shaderName(pShader), pShader->setCount, maxSets);
        return false;
    }

    // Names are how resources are found, so they have to be unambiguous
    for (uint32_t i = 0; i < pShader->bindingCount; i++) {
        for (uint32_t j = i + 1; j < pShader->bindingCount; j++) {
            const char* pName = bindingName(&pShader->pBindings[j]);
            if (!strcmp(bindingName(&pShader->pBindings[i]), pName)) {
                GFX_WARNING("%s declares '%s' at more than one binding, only set %" PRIu32 " binding %" PRIu32
                            " can be reached by name",
                            shaderName(pShader), pName, pShader->pBindings[i].set, pShader->pBindings[i].binding);
            }
        }
    }

    // Lay out the bindings and descriptor infos of each set contiguously
    pShader->pSets = allocZeroed(GFX_MAX(pShader->setCount, 1), sizeof *pShader->pSets);
    for (uint32_t i = 0; i < pShader->bindingCount; i++) {
        GfxShaderBinding* pBinding = &pShader->pBindings[i];
        GfxShaderSet* pSet = &pShader->pSets[pBinding->set];

        if (!pSet->bindingCount) {
            pSet->firstBinding = i;
            pSet->firstInfo = pShader->infoCount;
        }
        pSet->bindingCount++;
        pSet->infoCount += pBinding->count;

        pBinding->infoIndex = pShader->infoCount;
        pShader->infoCount += pBinding->count;
    }

    // Every set gets a layout, also the ones without bindings, since the
    // pipeline layout has to have one for every set up to the last
    VkDescriptorSetLayout* pSetLayouts = allocZeroed(GFX_MAX(pShader->setCount, 1), sizeof *pSetLayouts);
    VkDescriptorSetLayoutBinding* pLayoutBindings =
        allocZeroed(GFX_MAX(pShader->bindingCount, 1), sizeof *pLayoutBindings);

    for (uint32_t s = 0; s < pShader->setCount; s++) {
        GfxShaderSet* pSet = &pShader->pSets[s];

        for (uint32_t i = 0; i < pSet->bindingCount; i++) {
            const GfxShaderBinding* pBinding = &pShader->pBindings[pSet->firstBinding + i];
            pLayoutBindings[i] = (VkDescriptorSetLayoutBinding){
                .binding = pBinding->binding,
                .descriptorType = pBinding->type,
                .descriptorCount = pBinding->count,
                .stageFlags = pBinding->stages,
            };
        }

        VkDescriptorSetLayoutCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = pSet->bindingCount,
            .pBindings = pLayoutBindings,
        };
        VK_CHECK(vkCreateDescriptorSetLayout(gfxDevice.device, &ci, NULL, &pSet->layout));
        pSetLayouts[s] = pSet->layout;
    }

    VkPipelineLayoutCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = pShader->setCount,
        .pSetLayouts = pSetLayouts,
        .pushConstantRangeCount = pShader->pushConstantRange.size ? 1 : 0,
        .pPushConstantRanges = &pShader->pushConstantRange,
    };
    VK_CHECK(vkCreatePipelineLayout(gfxDevice.device, &ci, NULL, &pShader->pipelineLayout));

    GFX_FREE(pLayoutBindings);
    GFX_FREE(pSetLayouts);

    // Pool sizes for one generation of every set
    pShader->pPoolSizes = allocZeroed(GFX_MAX(pShader->bindingCount, 1), sizeof *pShader->pPoolSizes);
    for (uint32_t i = 0; i < pShader->bindingCount; i++) {
        const GfxShaderBinding* pBinding = &pShader->pBindings[i];
        uint32_t j = 0;
        while (j < pShader->poolSizeCount && pShader->pPoolSizes[j].type != pBinding->type) {
            j++;
        }
        if (j == pShader->poolSizeCount) {
            pShader->pPoolSizes[pShader->poolSizeCount++].type = pBinding->type;
        }
        pShader->pPoolSizes[j].descriptorCount += pBinding->count;
    }

    size_t infoCount = GFX_MAX(pShader->infoCount, 1);
    pShader->pWrittenBufferInfos = allocZeroed(infoCount, sizeof *pShader->pWrittenBufferInfos);
    pShader->pWrittenImageInfos = allocZeroed(infoCount, sizeof *pShader->pWrittenImageInfos);
    pShader->pResolvedBufferInfos = allocZeroed(infoCount, sizeof *pShader->pResolvedBufferInfos);
    pShader->pResolvedImageInfos = allocZeroed(infoCount, sizeof *pShader->pResolvedImageInfos);
    pShader->pWrittenAccelerationStructures = allocZeroed(infoCount, sizeof *pShader->pWrittenAccelerationStructures);
    pShader->pResolvedAccelerationStructures = allocZeroed(infoCount, sizeof *pShader->pResolvedAccelerationStructures);
    pShader->pWrites = allocZeroed(GFX_MAX(pShader->bindingCount, 1), sizeof *pShader->pWrites);
    pShader->pAccelerationStructureWrites =
        allocZeroed(GFX_MAX(pShader->bindingCount, 1), sizeof *pShader->pAccelerationStructureWrites);

    return true;
}

// The stage that follows a stage in the shader, or the fragment stage if the
// shader has none, so that the stage can be used with any fragment shader
static VkShaderStageFlags nextStage(VkShaderStageFlagBits stage, VkShaderStageFlags stages)
{
    switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
        if (stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        return (stages & VK_SHADER_STAGE_GEOMETRY_BIT) ? VK_SHADER_STAGE_GEOMETRY_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        return (stages & VK_SHADER_STAGE_GEOMETRY_BIT) ? VK_SHADER_STAGE_GEOMETRY_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
    case VK_SHADER_STAGE_GEOMETRY_BIT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case VK_SHADER_STAGE_TASK_BIT_EXT:
        return VK_SHADER_STAGE_MESH_BIT_EXT;
    case VK_SHADER_STAGE_MESH_BIT_EXT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    default:
        return 0;
    }
}

static bool buildShaderObjects(GfxShader* pShader, bool fatal)
{
    // Ray tracing stages are built into ray tracing pipelines instead
    if (pShader->stages & GFX_RAY_TRACING_STAGES) {
        return true;
    }

    VkDescriptorSetLayout* pSetLayouts = allocZeroed(GFX_MAX(pShader->setCount, 1), sizeof *pSetLayouts);
    for (uint32_t s = 0; s < pShader->setCount; s++) {
        pSetLayouts[s] = pShader->pSets[s].layout;
    }

    VkShaderCreateInfoEXT* pInfos = allocZeroed(pShader->stageCount, sizeof *pInfos);
    VkShaderEXT* pShaders = allocZeroed(pShader->stageCount, sizeof *pShaders);

    // Graphics stages are linked, which allows optimizing across them
    bool link = pShader->stageCount > 1;

    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        const GfxShaderStage* pStage = &pShader->pStages[i];

        VkShaderCreateFlagsEXT flags = link ? VK_SHADER_CREATE_LINK_STAGE_BIT_EXT : 0;
        if (pStage->stage == VK_SHADER_STAGE_MESH_BIT_EXT && !(pShader->stages & VK_SHADER_STAGE_TASK_BIT_EXT)) {
            flags |= VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
        }

        pInfos[i] = (VkShaderCreateInfoEXT){
            .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
            .flags = flags,
            .stage = pStage->stage,
            .nextStage = nextStage(pStage->stage, pShader->stages),
            .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
            .codeSize = pStage->codeSize,
            .pCode = pStage->pCode,
            .pName = "main",
            .setLayoutCount = pShader->setCount,
            .pSetLayouts = pSetLayouts,
            .pushConstantRangeCount = pShader->pushConstantRange.size ? 1 : 0,
            .pPushConstantRanges = &pShader->pushConstantRange,
            .pSpecializationInfo = pStage->pSpecializationInfo,
        };
    }

    VkResult result = gfxDevice.fn.vkCreateShadersEXT(gfxDevice.device, pShader->stageCount, pInfos, NULL, pShaders);

    bool ok = result >= 0;
    if (ok) {
        for (uint32_t i = 0; i < pShader->stageCount; i++) {
            pShader->pStages[i].shader = pShaders[i];
            const char* pName = pShader->pStages[i].pPath ? pShader->pStages[i].pPath : shaderName(pShader);
            gfxSetDebugName(VK_OBJECT_TYPE_SHADER_EXT, (uint64_t)pShaders[i], pName);
        }
    } else {
        GFX_FAIL(fatal, "Failed to create shader objects for %s: %s", shaderName(pShader), gfxResultString(result));
    }

    GFX_FREE(pShaders);
    GFX_FREE(pInfos);
    GFX_FREE(pSetLayouts);

    return ok;
}

static bool createShader(GfxShader* pShader, uint32_t stageCount, const GfxShaderStageDesc* pDescs, bool fatal)
{
    GFX_RESET(pShader);

    if (!gfxDevice.device) {
        GFX_ERROR("Device not initialized");
        return false;
    }

    if (!stageCount || !pDescs) {
        GFX_FAIL(fatal, "A shader needs at least one stage");
        return false;
    }

    pShader->stageCount = stageCount;
    pShader->pStages = allocZeroed(stageCount, sizeof *pShader->pStages);

    for (uint32_t i = 0; i < stageCount; i++) {
        if (!loadStage(&pShader->pStages[i], &pDescs[i], fatal)) {
            goto fail;
        }

        VkShaderStageFlagBits stage = pShader->pStages[i].stage;

        // Ray tracing pipelines can have several stages of the same kind
        if ((pShader->stages & stage) && !(stage & GFX_RAY_TRACING_STAGES)) {
            GFX_FAIL(fatal, "%s: the shader already has a stage of this kind", shaderName(pShader));
            goto fail;
        }
        pShader->stages |= stage;
    }

    if ((pShader->stages & VK_SHADER_STAGE_COMPUTE_BIT) && stageCount > 1) {
        GFX_FAIL(fatal, "%s: a compute stage must be on its own", shaderName(pShader));
        goto fail;
    }
    if ((pShader->stages & GFX_RAY_TRACING_STAGES) && (pShader->stages & ~GFX_RAY_TRACING_STAGES)) {
        GFX_FAIL(fatal, "%s: ray tracing stages cannot be combined with other stages", shaderName(pShader));
        goto fail;
    }

    uint32_t bindingCapacity = 0;
    for (uint32_t i = 0; i < stageCount; i++) {
        if (!reflectStage(pShader, &pShader->pStages[i], &bindingCapacity, fatal)) {
            goto fail;
        }
    }

    if (!createLayouts(pShader, fatal) || !buildShaderObjects(pShader, fatal)) {
        goto fail;
    }

    GFX_INFO("Created shader: %s", shaderName(pShader));
    for (uint32_t i = 1; i < stageCount; i++) {
        GFX_INFO("                %s", pShader->pStages[i].pPath ? pShader->pStages[i].pPath : "(SPIR-V)");
    }
#ifndef NDEBUG
    for (uint32_t i = 0; i < pShader->bindingCount; i++) {
        const GfxShaderBinding* pBinding = &pShader->pBindings[i];
        GFX_DEBUG(" * set %" PRIu32 ", binding %" PRIu32 ": %s '%s' (%" PRIu32 ")", pBinding->set, pBinding->binding,
                  descriptorTypeName(pBinding->type), bindingName(pBinding), pBinding->count);
    }
    if (pShader->pushConstantRange.size) {
        GFX_DEBUG(" * push constants: %" PRIu32 " bytes", pShader->pushConstantRange.size);
    }
#endif

    return true;

fail:
    gfxDestroyShader(pShader);
    return false;
}

void gfxCreateShader(uint32_t stageCount, const GfxShaderStageDesc* pStages, GfxShader* pShader)
{
    createShader(pShader, stageCount, pStages, true);
}

void gfxDestroyShader(GfxShader* pShader)
{
    vkDeviceWaitIdle(gfxDevice.device);

    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        GfxShaderStage* pStage = &pShader->pStages[i];
        if (pStage->shader) {
            gfxDevice.fn.vkDestroyShaderEXT(gfxDevice.device, pStage->shader, NULL);
        }
        GFX_FREE(pStage->pPath);
        GFX_FREE(pStage->pCode);
        GFX_FREE(pStage->pSpecializationInfo);
    }
    GFX_FREE(pShader->pStages);

    for (uint32_t i = 0; i < pShader->bindingCount; i++) {
        GFX_FREE(pShader->pBindings[i].pName);
        GFX_FREE(pShader->pBindings[i].pBlockName);
    }
    GFX_FREE(pShader->pBindings);

    // Sets are freed with their pools
    for (uint32_t i = 0; i < pShader->setCount; i++) {
        vkDestroyDescriptorSetLayout(gfxDevice.device, pShader->pSets[i].layout, NULL);
        GFX_FREE(pShader->pSets[i].pRetired);
    }
    GFX_FREE(pShader->pSets);

    vkDestroyPipelineLayout(gfxDevice.device, pShader->pipelineLayout, NULL);

    for (uint32_t i = 0; i < pShader->poolCount; i++) {
        vkDestroyDescriptorPool(gfxDevice.device, pShader->pPools[i], NULL);
    }
    GFX_FREE(pShader->pPools);
    GFX_FREE(pShader->pPoolSizes);

    GFX_FREE(pShader->pWrittenBufferInfos);
    GFX_FREE(pShader->pWrittenImageInfos);
    GFX_FREE(pShader->pResolvedBufferInfos);
    GFX_FREE(pShader->pResolvedImageInfos);
    GFX_FREE(pShader->pWrittenAccelerationStructures);
    GFX_FREE(pShader->pResolvedAccelerationStructures);
    GFX_FREE(pShader->pWrites);
    GFX_FREE(pShader->pAccelerationStructureWrites);

    GFX_RESET(pShader);
}

static GfxShaderBinding* findBinding(const GfxShader* pShader, const char* pName)
{
    for (uint32_t i = 0; i < pShader->bindingCount; i++) {
        GfxShaderBinding* pBinding = &pShader->pBindings[i];
        if ((pBinding->pName[0] && !strcmp(pBinding->pName, pName)) ||
            (pBinding->pBlockName && !strcmp(pBinding->pBlockName, pName))) {
            return pBinding;
        }
    }
    return NULL;
}

bool gfxReloadShader(GfxShader* pShader)
{
    // Recreate from the same descriptions. They point into the current shader,
    // which is left alone until the new one has been created.
    GfxShaderStageDesc* pDescs = allocZeroed(pShader->stageCount, sizeof *pDescs);
    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        const GfxShaderStage* pStage = &pShader->pStages[i];
        pDescs[i] = (GfxShaderStageDesc){
            .pPath = pStage->pPath,
            .pCode = pStage->pPath ? NULL : pStage->pCode,
            .codeSize = pStage->pPath ? 0 : pStage->codeSize,
            .stage = pStage->stage,
            .pSpecializationInfo = pStage->pSpecializationInfo,
        };
    }

    GfxShader reloaded;
    bool ok = createShader(&reloaded, pShader->stageCount, pDescs, false);
    GFX_FREE(pDescs);

    if (!ok) {
        GFX_WARNING("Reloading %s failed, keeping the previous version", shaderName(pShader));
        return false;
    }

    // Attach the resources again by name, since they may have moved to other
    // sets and bindings
    for (uint32_t i = 0; i < pShader->bindingCount; i++) {
        const GfxShaderBinding* pOld = &pShader->pBindings[i];
        if (!pOld->pBuffer && !pOld->pTextures && !pOld->pImage && !pOld->pAccelerationStructure) {
            continue;
        }

        GfxShaderBinding* pNew = findBinding(&reloaded, bindingName(pOld));
        if (!pNew || pNew->type != pOld->type || pNew->count != pOld->count) {
            GFX_WARNING("'%s' no longer matches a resource in %s and is not attached", bindingName(pOld),
                        shaderName(pShader));
            continue;
        }

        pNew->pBuffer = pOld->pBuffer;
        pNew->offset = pOld->offset;
        pNew->range = pOld->range;
        pNew->pTextures = pOld->pTextures;
        pNew->pImage = pOld->pImage;
        pNew->imageLayout = pOld->imageLayout;
        pNew->sampler = pOld->sampler;
        pNew->pAccelerationStructure = pOld->pAccelerationStructure;
    }

    // Lets pipelines built from the shader notice that it changed
    reloaded.version = pShader->version + 1;

    gfxDestroyShader(pShader);
    *pShader = reloaded;

    return true;
}

const GfxShaderBinding* gfxGetShaderBinding(const GfxShader* pShader, const char* pName)
{
    return findBinding(pShader, pName);
}

// Look up a binding and check that it takes the kind of resource being attached
static GfxShaderBinding* resolveBinding(GfxShader* pShader, const char* pName, const char* pKind,
                                        const VkDescriptorType* pAllowed, uint32_t allowedCount)
{
    GfxShaderBinding* pBinding = findBinding(pShader, pName);
    if (!pBinding) {
        GFX_ERROR("%s has no resource named '%s'. A block declared without an instance name is reached through its "
                  "block type name.",
                  shaderName(pShader), pName);
        return NULL;
    }

    for (uint32_t i = 0; i < allowedCount; i++) {
        if (pBinding->type == pAllowed[i]) {
            // Only one resource is attached at a time
            pBinding->pBuffer = NULL;
            pBinding->pTextures = NULL;
            pBinding->pImage = NULL;
            pBinding->sampler = VK_NULL_HANDLE;
            pBinding->pAccelerationStructure = NULL;
            return pBinding;
        }
    }

    GFX_ERROR("'%s' is a %s in %s and cannot be set from a %s", pName, descriptorTypeName(pBinding->type),
              shaderName(pShader), pKind);
    return NULL;
}

void gfxSetShaderBuffer(GfxShader* pShader, const char* pName, const GfxBuffer* pBuffer)
{
    gfxSetShaderBufferRange(pShader, pName, pBuffer, 0, VK_WHOLE_SIZE);
}

void gfxSetShaderBufferRange(GfxShader* pShader, const char* pName, const GfxBuffer* pBuffer, VkDeviceSize offset,
                             VkDeviceSize range)
{
    static const VkDescriptorType allowed[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    GfxShaderBinding* pBinding = resolveBinding(pShader, pName, "buffer", allowed, GFX_ARRAY_LEN(allowed));
    if (!pBinding) {
        return;
    }

    if (pBinding->count != 1) {
        GFX_ERROR("'%s' is an array of %" PRIu32 " buffers, which cannot be attached", pName, pBinding->count);
        return;
    }

    VkBufferUsageFlags usage = pBinding->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                                                                   : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (!(pBuffer->usage & usage)) {
        GFX_ERROR("'%s' is a %s, but the buffer was not created with that usage", pName,
                  descriptorTypeName(pBinding->type));
        return;
    }

    pBinding->pBuffer = pBuffer;
    pBinding->offset = offset;
    pBinding->range = range;
}

void gfxSetShaderTexture(GfxShader* pShader, const char* pName, const GfxTexture* pTexture)
{
    gfxSetShaderTextureArray(pShader, pName, 1, pTexture);
}

void gfxSetShaderTextureArray(GfxShader* pShader, const char* pName, uint32_t textureCount, const GfxTexture* pTextures)
{
    static const VkDescriptorType allowed[] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                               VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_SAMPLER};

    GfxShaderBinding* pBinding = resolveBinding(pShader, pName, "texture", allowed, GFX_ARRAY_LEN(allowed));
    if (!pBinding) {
        return;
    }

    if (pBinding->count != textureCount) {
        GFX_ERROR("'%s' has %" PRIu32 " descriptors in the shader, but %" PRIu32 " textures were given", pName,
                  pBinding->count, textureCount);
        return;
    }

    pBinding->pTextures = pTextures;
}

void gfxSetShaderImageSampler(GfxShader* pShader, const char* pName, const GfxImage* pImage, VkSampler sampler,
                              VkImageLayout layout)
{
    static const VkDescriptorType allowed[] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

    GfxShaderBinding* pBinding = resolveBinding(pShader, pName, "image and sampler", allowed, GFX_ARRAY_LEN(allowed));
    if (!pBinding) {
        return;
    }

    if (pBinding->count != 1) {
        GFX_ERROR("'%s' is an array of %" PRIu32 " images, which cannot be attached", pName, pBinding->count);
        return;
    }

    pBinding->pImage = pImage;
    pBinding->sampler = sampler;
    pBinding->imageLayout = layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : layout;
}

void gfxSetShaderImage(GfxShader* pShader, const char* pName, const GfxImage* pImage, VkImageLayout layout)
{
    static const VkDescriptorType allowed[] = {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                               VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT};

    GfxShaderBinding* pBinding = resolveBinding(pShader, pName, "image", allowed, GFX_ARRAY_LEN(allowed));
    if (!pBinding) {
        return;
    }

    if (pBinding->count != 1) {
        GFX_ERROR("'%s' is an array of %" PRIu32 " images, which cannot be attached", pName, pBinding->count);
        return;
    }

    if (layout == VK_IMAGE_LAYOUT_UNDEFINED) {
        layout = pBinding->type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ? VK_IMAGE_LAYOUT_GENERAL
                                                                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    pBinding->pImage = pImage;
    pBinding->imageLayout = layout;
}

void gfxSetShaderAccelerationStructure(GfxShader* pShader, const char* pName,
                                       const GfxAccelerationStructure* pAccelerationStructure)
{
    static const VkDescriptorType allowed[] = {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR};

    GfxShaderBinding* pBinding =
        resolveBinding(pShader, pName, "acceleration structure", allowed, GFX_ARRAY_LEN(allowed));
    if (!pBinding) {
        return;
    }

    if (pBinding->count != 1) {
        GFX_ERROR("'%s' is an array of %" PRIu32 " acceleration structures, which cannot be attached", pName,
                  pBinding->count);
        return;
    }

    if (pAccelerationStructure->type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
        GFX_ERROR("Only top level acceleration structures can be attached to '%s'", pName);
        return;
    }

    pBinding->pAccelerationStructure = pAccelerationStructure;
}

void gfxCmdBindShader(VkCommandBuffer cmd, const GfxShader* pShader)
{
    if (pShader->stages & GFX_RAY_TRACING_STAGES) {
        GFX_ERROR("%s has ray tracing stages, which are used through a ray tracing pipeline", shaderName(pShader));
        return;
    }

    static const VkShaderStageFlagBits graphicsStages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        VK_SHADER_STAGE_GEOMETRY_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        VK_SHADER_STAGE_TASK_BIT_EXT,
        VK_SHADER_STAGE_MESH_BIT_EXT,
    };

    // A shader has at most one of each graphics stage, or a single compute stage
    VkShaderStageFlagBits stages[GFX_ARRAY_LEN(graphicsStages)];
    VkShaderEXT shaders[GFX_ARRAY_LEN(graphicsStages)];
    uint32_t count = 0;

    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        stages[count] = pShader->pStages[i].stage;
        shaders[count++] = pShader->pStages[i].shader;
    }

    // Graphics stages the shader does not have are unbound, so that nothing
    // from a previously bound shader is left behind
    if (!(pShader->stages & VK_SHADER_STAGE_COMPUTE_BIT)) {
        for (uint32_t i = 0; i < GFX_ARRAY_LEN(graphicsStages); i++) {
            if ((gfxDevice.graphicsShaderStages & graphicsStages[i]) && !(pShader->stages & graphicsStages[i])) {
                stages[count] = graphicsStages[i];
                shaders[count++] = VK_NULL_HANDLE;
            }
        }
    }

    gfxDevice.fn.vkCmdBindShadersEXT(cmd, count, stages, shaders);
}

static VkPipelineBindPoint shaderBindPoint(const GfxShader* pShader)
{
    if (pShader->stages & VK_SHADER_STAGE_COMPUTE_BIT) {
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    }
    if (pShader->stages & GFX_RAY_TRACING_STAGES) {
        return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
    }
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

// Turn what is attached to the bindings of a set into descriptor infos.
// Bindings without a usable resource are left zeroed and are not written.
// Returns the number of bindings that will be written.
static uint32_t resolveSet(GfxShader* pShader, const GfxShaderSet* pSet)
{
    memset(&pShader->pResolvedBufferInfos[pSet->firstInfo], 0, pSet->infoCount * sizeof *pShader->pResolvedBufferInfos);
    memset(&pShader->pResolvedImageInfos[pSet->firstInfo], 0, pSet->infoCount * sizeof *pShader->pResolvedImageInfos);
    memset(&pShader->pResolvedAccelerationStructures[pSet->firstInfo], 0,
           pSet->infoCount * sizeof *pShader->pResolvedAccelerationStructures);

    uint32_t resolved = 0;

    for (uint32_t i = 0; i < pSet->bindingCount; i++) {
        GfxShaderBinding* pBinding = &pShader->pBindings[pSet->firstBinding + i];
        VkDescriptorBufferInfo* pBufferInfo = &pShader->pResolvedBufferInfos[pBinding->infoIndex];
        VkDescriptorImageInfo* pImageInfos = &pShader->pResolvedImageInfos[pBinding->infoIndex];

        if (pBinding->pBuffer) {
            const GfxBuffer* pBuffer = pBinding->pBuffer;
            if (!pBuffer->buffer) {
                GFX_ERROR("The buffer attached to '%s' has been destroyed", bindingName(pBinding));
                continue;
            }
            pBufferInfo->buffer = pBuffer->buffer;
            pBufferInfo->offset = pBinding->offset;
            pBufferInfo->range = pBinding->range;

            // A per-frame buffer is bound at the copy of the frame being recorded
            if (pBuffer->frameCount) {
                if (pBufferInfo->range == VK_WHOLE_SIZE) {
                    pBufferInfo->range = pBuffer->frameSize - pBinding->offset;
                }
                pBufferInfo->offset += gfxGetPerFrameOffset(pBuffer);
            }
            resolved++;
        } else if (pBinding->pTextures) {
            bool valid = true;
            for (uint32_t j = 0; j < pBinding->count; j++) {
                const GfxTexture* pTexture = &pBinding->pTextures[j];
                if (!pTexture->sampler || !pTexture->image.imageView) {
                    valid = false;
                    break;
                }
                pImageInfos[j].sampler = pTexture->sampler;
                pImageInfos[j].imageView = pTexture->image.imageView;
                pImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
            if (!valid) {
                GFX_ERROR("A texture attached to '%s' has been destroyed", bindingName(pBinding));
                memset(pImageInfos, 0, pBinding->count * sizeof *pImageInfos);
                continue;
            }
            resolved++;
        } else if (pBinding->pImage) {
            if (!pBinding->pImage->imageView) {
                GFX_ERROR("The image attached to '%s' has been destroyed or has no image view", bindingName(pBinding));
                continue;
            }
            pImageInfos[0].sampler = pBinding->sampler;
            pImageInfos[0].imageView = pBinding->pImage->imageView;
            pImageInfos[0].imageLayout = pBinding->imageLayout;
            resolved++;
        } else if (pBinding->pAccelerationStructure) {
            if (!pBinding->pAccelerationStructure->accelerationStructure) {
                GFX_ERROR("The acceleration structure attached to '%s' has been destroyed", bindingName(pBinding));
                continue;
            }
            pShader->pResolvedAccelerationStructures[pBinding->infoIndex] =
                pBinding->pAccelerationStructure->accelerationStructure;
            resolved++;
        } else if (!pBinding->reported) {
            GFX_WARNING("%s: '%s' (set %" PRIu32 ", binding %" PRIu32 ") has nothing attached", shaderName(pShader),
                        bindingName(pBinding), pBinding->set, pBinding->binding);
            pBinding->reported = true;
        }
    }

    return resolved;
}

static bool setChanged(const GfxShader* pShader, const GfxShaderSet* pSet)
{
    if (!pSet->current || pSet->generation != gfxDevice.resourceGeneration) {
        return true;
    }

    return memcmp(&pShader->pResolvedBufferInfos[pSet->firstInfo], &pShader->pWrittenBufferInfos[pSet->firstInfo],
                  pSet->infoCount * sizeof *pShader->pResolvedBufferInfos) ||
           memcmp(&pShader->pResolvedImageInfos[pSet->firstInfo], &pShader->pWrittenImageInfos[pSet->firstInfo],
                  pSet->infoCount * sizeof *pShader->pResolvedImageInfos) ||
           memcmp(&pShader->pResolvedAccelerationStructures[pSet->firstInfo],
                  &pShader->pWrittenAccelerationStructures[pSet->firstInfo],
                  pSet->infoCount * sizeof *pShader->pResolvedAccelerationStructures);
}

static void createDescriptorPool(GfxShader* pShader)
{
    VkDescriptorPoolSize* pSizes = allocZeroed(GFX_MAX(pShader->poolSizeCount, 1), sizeof *pSizes);
    for (uint32_t i = 0; i < pShader->poolSizeCount; i++) {
        pSizes[i] = (VkDescriptorPoolSize){
            .type = pShader->pPoolSizes[i].type,
            .descriptorCount = pShader->pPoolSizes[i].descriptorCount * GFX_DESCRIPTOR_SETS_PER_POOL,
        };
    }

    VkDescriptorPoolCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = pShader->setCount * GFX_DESCRIPTOR_SETS_PER_POOL,
        .poolSizeCount = pShader->poolSizeCount,
        .pPoolSizes = pSizes,
    };

    VkDescriptorPool pool;
    VK_CHECK(vkCreateDescriptorPool(gfxDevice.device, &ci, NULL, &pool));
    GFX_FREE(pSizes);

    pShader->pPools = GFX_REALLOC(pShader->pPools, (pShader->poolCount + 1) * sizeof *pShader->pPools);
    if (!pShader->pPools) {
        GFX_ERROR("Unable to allocate memory");
    }
    pShader->pPools[pShader->poolCount++] = pool;
}

// Get a descriptor set that no frame in flight can be using: a retired one
// whose last frame has finished, or a newly allocated one
static VkDescriptorSet acquireDescriptorSet(GfxShader* pShader, GfxShaderSet* pSet)
{
    for (uint32_t i = 0; i < pSet->retiredCount; i++) {
        if (pSet->pRetired[i].frame < gfxDevice.completedFrames) {
            VkDescriptorSet set = pSet->pRetired[i].set;
            pSet->pRetired[i] = pSet->pRetired[--pSet->retiredCount];
            return set;
        }
    }

    if (!pShader->poolCount) {
        createDescriptorPool(pShader);
    }

    VkDescriptorSetAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pShader->pPools[pShader->poolCount - 1],
        .descriptorSetCount = 1,
        .pSetLayouts = &pSet->layout,
    };

    VkDescriptorSet set = VK_NULL_HANDLE;
    VkResult result = vkAllocateDescriptorSets(gfxDevice.device, &ai, &set);

    // Sets are never freed back to their pool, so a full pool is followed by another
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        createDescriptorPool(pShader);
        ai.descriptorPool = pShader->pPools[pShader->poolCount - 1];
        result = vkAllocateDescriptorSets(gfxDevice.device, &ai, &set);
    }

    VK_CHECK(result);
    return set;
}

// Keep a set that has been replaced until the frame being recorded, which may
// still use it, has finished
static void retireDescriptorSet(GfxShaderSet* pSet)
{
    if (!pSet->current) {
        return;
    }

    if (pSet->retiredCount == pSet->retiredCapacity) {
        pSet->retiredCapacity = pSet->retiredCapacity ? pSet->retiredCapacity * 2 : 4;
        pSet->pRetired = GFX_REALLOC(pSet->pRetired, pSet->retiredCapacity * sizeof *pSet->pRetired);
        if (!pSet->pRetired) {
            GFX_ERROR("Unable to allocate memory");
        }
    }

    pSet->pRetired[pSet->retiredCount++] = (GfxRetiredDescriptorSet){
        .set = pSet->current,
        .frame = gfxDevice.frame,
    };
    pSet->current = VK_NULL_HANDLE;
}

static void writeDescriptorSet(GfxShader* pShader, GfxShaderSet* pSet)
{
    memcpy(&pShader->pWrittenBufferInfos[pSet->firstInfo], &pShader->pResolvedBufferInfos[pSet->firstInfo],
           pSet->infoCount * sizeof *pShader->pWrittenBufferInfos);
    memcpy(&pShader->pWrittenImageInfos[pSet->firstInfo], &pShader->pResolvedImageInfos[pSet->firstInfo],
           pSet->infoCount * sizeof *pShader->pWrittenImageInfos);
    memcpy(&pShader->pWrittenAccelerationStructures[pSet->firstInfo],
           &pShader->pResolvedAccelerationStructures[pSet->firstInfo],
           pSet->infoCount * sizeof *pShader->pWrittenAccelerationStructures);

    uint32_t writeCount = 0;
    for (uint32_t i = 0; i < pSet->bindingCount; i++) {
        const GfxShaderBinding* pBinding = &pShader->pBindings[pSet->firstBinding + i];
        const VkDescriptorBufferInfo* pBufferInfo = &pShader->pWrittenBufferInfos[pBinding->infoIndex];
        const VkDescriptorImageInfo* pImageInfo = &pShader->pWrittenImageInfos[pBinding->infoIndex];

        VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = pSet->current,
            .dstBinding = pBinding->binding,
            .dstArrayElement = 0,
            .descriptorCount = pBinding->count,
            .descriptorType = pBinding->type,
        };

        // Bindings that did not resolve are left unwritten
        if (isBufferDescriptor(pBinding->type) && pBufferInfo->buffer) {
            write.pBufferInfo = pBufferInfo;
        } else if (isImageDescriptor(pBinding->type) && (pImageInfo->imageView || pImageInfo->sampler)) {
            write.pImageInfo = pImageInfo;
        } else if (pBinding->type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR &&
                   pShader->pWrittenAccelerationStructures[pBinding->infoIndex]) {
            VkWriteDescriptorSetAccelerationStructureKHR* pAccelerationStructureWrite =
                &pShader->pAccelerationStructureWrites[writeCount];
            *pAccelerationStructureWrite = (VkWriteDescriptorSetAccelerationStructureKHR){
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
                .accelerationStructureCount = pBinding->count,
                .pAccelerationStructures = &pShader->pWrittenAccelerationStructures[pBinding->infoIndex],
            };
            write.pNext = pAccelerationStructureWrite;
        } else {
            continue;
        }

        pShader->pWrites[writeCount++] = write;
    }

    vkUpdateDescriptorSets(gfxDevice.device, writeCount, pShader->pWrites, 0, NULL);

    pSet->generation = gfxDevice.resourceGeneration;
}

void gfxCmdBindResources(VkCommandBuffer cmd, GfxShader* pShader)
{
    VkPipelineBindPoint bindPoint = shaderBindPoint(pShader);

    for (uint32_t s = 0; s < pShader->setCount; s++) {
        GfxShaderSet* pSet = &pShader->pSets[s];

        if (!pSet->bindingCount || !resolveSet(pShader, pSet)) {
            continue;
        }

        // A set is never written while a frame in flight may read it, a new
        // one is taken instead
        if (setChanged(pShader, pSet)) {
            retireDescriptorSet(pSet);
            pSet->current = acquireDescriptorSet(pShader, pSet);
            writeDescriptorSet(pShader, pSet);
        }

        vkCmdBindDescriptorSets(cmd, bindPoint, pShader->pipelineLayout, s, 1, &pSet->current, 0, NULL);
    }
}

void gfxCmdPushConstants(VkCommandBuffer cmd, const GfxShader* pShader, uint32_t offset, uint32_t size,
                         const void* pData)
{
    if ((uint64_t)offset + size > pShader->pushConstantRange.size) {
        GFX_ERROR("Pushing %" PRIu32 " bytes at offset %" PRIu32 ", but %s only has %" PRIu32
                  " bytes of push constants",
                  size, offset, shaderName(pShader), pShader->pushConstantRange.size);
        return;
    }

    vkCmdPushConstants(cmd, pShader->pipelineLayout, pShader->pushConstantRange.stageFlags, offset, size, pData);
}

void gfxCmdDispatch(VkCommandBuffer cmd, const GfxShader* pShader, uint32_t x, uint32_t y, uint32_t z)
{
    if (!(pShader->stages & VK_SHADER_STAGE_COMPUTE_BIT)) {
        GFX_ERROR("%s is not a compute shader", shaderName(pShader));
        return;
    }

    // A local size that could not be reflected is taken as 1
    uint32_t groupsX = (x + GFX_MAX(pShader->localSize[0], 1) - 1) / GFX_MAX(pShader->localSize[0], 1);
    uint32_t groupsY = (y + GFX_MAX(pShader->localSize[1], 1) - 1) / GFX_MAX(pShader->localSize[1], 1);
    uint32_t groupsZ = (z + GFX_MAX(pShader->localSize[2], 1) - 1) / GFX_MAX(pShader->localSize[2], 1);
    vkCmdDispatch(cmd, groupsX, groupsY, groupsZ);
}

// Acceleration structures //

// Scratch memory has to be aligned beyond what buffers are, so the buffer is
// made larger and the address aligned up
static void createScratchBuffer(VkDeviceSize size, GfxBuffer* pScratch)
{
    VkDeviceSize alignment =
        GFX_MAX(gfxDevice.properties.accelerationStructure.minAccelerationStructureScratchOffsetAlignment, 1);
    gfxCreateBuffer(size + alignment, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pScratch);
}

static VkDeviceAddress scratchAddress(const GfxBuffer* pScratch)
{
    VkDeviceSize alignment =
        GFX_MAX(gfxDevice.properties.accelerationStructure.minAccelerationStructureScratchOffsetAlignment, 1);
    return gfxAlignTo(gfxGetBufferDeviceAddress(pScratch), alignment);
}

static void createAccelerationStructure(GfxAccelerationStructure* pAccelerationStructure,
                                        VkAccelerationStructureTypeKHR type, VkDeviceSize size)
{
    gfxCreateBuffer(size,
                    VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pAccelerationStructure->buffer);

    VkAccelerationStructureCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .buffer = pAccelerationStructure->buffer.buffer,
        .offset = 0,
        .size = size,
        .type = type,
    };
    VK_CHECK(gfxDevice.fn.vkCreateAccelerationStructureKHR(gfxDevice.device, &ci, NULL,
                                                           &pAccelerationStructure->accelerationStructure));

    VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
        .accelerationStructure = pAccelerationStructure->accelerationStructure,
    };
    pAccelerationStructure->address =
        gfxDevice.fn.vkGetAccelerationStructureDeviceAddressKHR(gfxDevice.device, &addressInfo);
    pAccelerationStructure->type = type;
}

static bool checkBuildInput(const GfxBuffer* pBuffer, const char* pKind)
{
    const VkBufferUsageFlags required = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

    if (!pBuffer || (pBuffer->usage & required) != required) {
        GFX_ERROR("The %s buffer of an acceleration structure needs VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT and "
                  "VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR",
                  pKind);
        return false;
    }
    return true;
}

void gfxCreateBLAS(uint32_t geometryCount, const GfxGeometryDesc* pGeometries,
                   VkBuildAccelerationStructureFlagsKHR flags, GfxAccelerationStructure* pBLAS)
{
    GFX_RESET(pBLAS);

    if (!gfxDevice.accelerationStructures) {
        GFX_ERROR("Acceleration structures need VK_KHR_acceleration_structure to be enabled");
        return;
    }

    if (!geometryCount) {
        GFX_ERROR("A bottom level acceleration structure needs at least one geometry");
        return;
    }

    VkAccelerationStructureGeometryKHR* pGeometryInfos = allocZeroed(geometryCount, sizeof *pGeometryInfos);
    VkAccelerationStructureBuildRangeInfoKHR* pRanges = allocZeroed(geometryCount, sizeof *pRanges);
    uint32_t* pPrimitiveCounts = allocZeroed(geometryCount, sizeof *pPrimitiveCounts);

    for (uint32_t i = 0; i < geometryCount; i++) {
        const GfxGeometryDesc* pDesc = &pGeometries[i];

        if (!checkBuildInput(pDesc->pVertexBuffer, "vertex") ||
            (pDesc->pIndexBuffer && !checkBuildInput(pDesc->pIndexBuffer, "index"))) {
            goto done;
        }

        VkDeviceAddress vertexAddress = gfxGetBufferDeviceAddress(pDesc->pVertexBuffer) + pDesc->vertexOffset;
        VkDeviceAddress indexAddress =
            pDesc->pIndexBuffer ? gfxGetBufferDeviceAddress(pDesc->pIndexBuffer) + pDesc->indexOffset : 0;

        pGeometryInfos[i] = (VkAccelerationStructureGeometryKHR){
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
            .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
            .geometry.triangles =
                {
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                    .vertexFormat = pDesc->vertexFormat ? pDesc->vertexFormat : VK_FORMAT_R32G32B32_SFLOAT,
                    .vertexData.deviceAddress = vertexAddress,
                    .vertexStride = pDesc->vertexStride,
                    .maxVertex = pDesc->vertexCount ? pDesc->vertexCount - 1 : 0,
                    .indexType = pDesc->pIndexBuffer ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_NONE_KHR,
                    .indexData.deviceAddress = indexAddress,
                },
            .flags = pDesc->flags,
        };

        pPrimitiveCounts[i] = (pDesc->pIndexBuffer ? pDesc->indexCount : pDesc->vertexCount) / 3;
        pRanges[i].primitiveCount = pPrimitiveCounts[i];
        pBLAS->primitiveCount += pPrimitiveCounts[i];
    }

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .flags = flags,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .geometryCount = geometryCount,
        .pGeometries = pGeometryInfos,
    };

    VkAccelerationStructureBuildSizesInfoKHR sizes = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
    };
    gfxDevice.fn.vkGetAccelerationStructureBuildSizesKHR(
        gfxDevice.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, pPrimitiveCounts, &sizes);

    pBLAS->flags = flags;
    createAccelerationStructure(pBLAS, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                                sizes.accelerationStructureSize);

    // Bottom level structures are not updated, so the scratch memory is only
    // needed for the build, and is released when the upload batch has finished
    GfxBuffer scratch;
    createScratchBuffer(sizes.buildScratchSize, &scratch);

    buildInfo.dstAccelerationStructure = pBLAS->accelerationStructure;
    buildInfo.scratchData.deviceAddress = scratchAddress(&scratch);

    VkCommandBuffer cmd = uploadCommandBuffer();
    beforeUploadBuild();
    const VkAccelerationStructureBuildRangeInfoKHR* pRangeInfos = pRanges;
    gfxDevice.fn.vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &pRangeInfos);

    retireBuffer(&scratch);

done:
    GFX_FREE(pPrimitiveCounts);
    GFX_FREE(pRanges);
    GFX_FREE(pGeometryInfos);
}

// Record a build or update of a top level acceleration structure, with the
// barriers that order it after earlier and before later use of the structure
static void cmdBuildTLAS(VkCommandBuffer cmd, GfxAccelerationStructure* pTLAS, uint32_t instanceCount,
                         const VkAccelerationStructureInstanceKHR* pInstances, bool update)
{
    // Earlier traces may still read the structure and the instances, and an
    // earlier build may still use the scratch memory
    gfxMemoryBarrier(cmd, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                     VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
                     VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                     VK_ACCESS_2_TRANSFER_WRITE_BIT | VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR |
                         VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR);

    // The instances are recorded into the command buffer, so the host never
    // writes memory that a frame in flight reads. Updates are limited to 64 kB
    // each.
    const VkDeviceSize maxUpdateSize = 65536;
    VkDeviceSize size = (VkDeviceSize)instanceCount * sizeof *pInstances;
    for (VkDeviceSize offset = 0; offset < size; offset += maxUpdateSize) {
        vkCmdUpdateBuffer(cmd, pTLAS->instances.buffer, offset, GFX_MIN(size - offset, maxUpdateSize),
                          (const char*)pInstances + offset);
    }

    gfxMemoryBarrier(cmd, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                     VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                     VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_SHADER_READ_BIT);

    VkAccelerationStructureGeometryKHR geometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
        .geometry.instances =
            {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                .arrayOfPointers = VK_FALSE,
                .data.deviceAddress = gfxGetBufferDeviceAddress(&pTLAS->instances),
            },
    };

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .flags = pTLAS->flags,
        .mode =
            update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .srcAccelerationStructure = update ? pTLAS->accelerationStructure : VK_NULL_HANDLE,
        .dstAccelerationStructure = pTLAS->accelerationStructure,
        .geometryCount = 1,
        .pGeometries = &geometry,
        .scratchData.deviceAddress = scratchAddress(&pTLAS->scratch),
    };

    VkAccelerationStructureBuildRangeInfoKHR range = {.primitiveCount = instanceCount};
    const VkAccelerationStructureBuildRangeInfoKHR* pRange = &range;
    gfxDevice.fn.vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &pRange);

    gfxMemoryBarrier(cmd, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                     VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                     VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR);

    pTLAS->primitiveCount = instanceCount;
}

void gfxCreateTLAS(uint32_t instanceCount, const VkAccelerationStructureInstanceKHR* pInstances,
                   VkBuildAccelerationStructureFlagsKHR flags, GfxAccelerationStructure* pTLAS)
{
    GFX_RESET(pTLAS);

    if (!gfxDevice.accelerationStructures) {
        GFX_ERROR("Acceleration structures need VK_KHR_acceleration_structure to be enabled");
        return;
    }

    pTLAS->flags = flags;
    pTLAS->instanceCapacity = GFX_MAX(instanceCount, 1);

    gfxCreateBuffer(pTLAS->instanceCapacity * sizeof(VkAccelerationStructureInstanceKHR),
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pTLAS->instances);

    // Size the structure for as many instances as it is created with
    VkAccelerationStructureGeometryKHR geometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
        .geometry.instances =
            {
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                .data.deviceAddress = gfxGetBufferDeviceAddress(&pTLAS->instances),
            },
    };

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .flags = flags,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .geometryCount = 1,
        .pGeometries = &geometry,
    };

    VkAccelerationStructureBuildSizesInfoKHR sizes = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
    };
    gfxDevice.fn.vkGetAccelerationStructureBuildSizesKHR(gfxDevice.device,
                                                         VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
                                                         &pTLAS->instanceCapacity, &sizes);

    createAccelerationStructure(pTLAS, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, sizes.accelerationStructureSize);

    // The scratch memory is kept, since later updates and rebuilds need it
    createScratchBuffer(GFX_MAX(sizes.buildScratchSize, sizes.updateScratchSize), &pTLAS->scratch);

    // The build starts with a barrier after the builds of the bottom level
    // structures it holds
    VkCommandBuffer cmd = uploadCommandBuffer();
    beforeUploadBuild();
    cmdBuildTLAS(cmd, pTLAS, instanceCount, pInstances, false);
}

void gfxCmdUpdateTLAS(VkCommandBuffer cmd, GfxAccelerationStructure* pTLAS, uint32_t instanceCount,
                      const VkAccelerationStructureInstanceKHR* pInstances)
{
    if (pTLAS->type != VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
        GFX_ERROR("Only top level acceleration structures can be updated");
        return;
    }

    if (instanceCount > pTLAS->instanceCapacity) {
        GFX_ERROR("Updating with %" PRIu32 " instances, but the acceleration structure was created for %" PRIu32,
                  instanceCount, pTLAS->instanceCapacity);
        return;
    }

    // Refitting keeps the structure of the previous build, which is only
    // possible when it was allowed and the instance count is the same
    bool update =
        (pTLAS->flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR) && instanceCount == pTLAS->primitiveCount;

    cmdBuildTLAS(cmd, pTLAS, instanceCount, pInstances, update);
}

void gfxDestroyAccelerationStructure(GfxAccelerationStructure* pAccelerationStructure)
{
    // A pending upload may still build the structure
    gfxFlushUploads(true);
    vkDeviceWaitIdle(gfxDevice.device);

    if (pAccelerationStructure->accelerationStructure) {
        gfxDevice.fn.vkDestroyAccelerationStructureKHR(gfxDevice.device, pAccelerationStructure->accelerationStructure,
                                                       NULL);
    }

    GfxBuffer* buffers[] = {&pAccelerationStructure->buffer, &pAccelerationStructure->scratch,
                            &pAccelerationStructure->instances};
    for (uint32_t i = 0; i < GFX_ARRAY_LEN(buffers); i++) {
        if (buffers[i]->buffer) {
            gfxDestroyBuffer(buffers[i]);
        }
    }

    gfxDevice.resourceGeneration++;

    GFX_RESET(pAccelerationStructure);
}


// Ray tracing pipelines //

static VkRayTracingShaderGroupCreateInfoKHR generalShaderGroup(uint32_t stage)
{
    return (VkRayTracingShaderGroupCreateInfoKHR){
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
        .type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
        .generalShader = stage,
        .closestHitShader = VK_SHADER_UNUSED_KHR,
        .anyHitShader = VK_SHADER_UNUSED_KHR,
        .intersectionShader = VK_SHADER_UNUSED_KHR,
    };
}

static VkStridedDeviceAddressRegionKHR shaderBindingTableRegion(VkDeviceAddress address, VkDeviceSize stride,
                                                                uint32_t count)
{
    if (!count) {
        return (VkStridedDeviceAddressRegionKHR){0};
    }
    return (VkStridedDeviceAddressRegionKHR){
        .deviceAddress = address,
        .stride = stride,
        .size = stride * count,
    };
}

static void createRayTracingPipeline(GfxRayTracingPipeline* pPipeline)
{
    const GfxShader* pShader = pPipeline->pShader;
    const VkPhysicalDeviceRayTracingPipelinePropertiesKHR* pProperties = &gfxDevice.properties.rayTracingPipeline;

    if (pPipeline->maxRecursionDepth > pProperties->maxRayRecursionDepth) {
        GFX_ERROR("A recursion depth of %" PRIu32 " was asked for, but the device supports %" PRIu32,
                  pPipeline->maxRecursionDepth, pProperties->maxRayRecursionDepth);
        return;
    }

    VkPipelineShaderStageCreateInfo* pStages = allocZeroed(pShader->stageCount, sizeof *pStages);
    VkShaderModule* pModules = allocZeroed(pShader->stageCount, sizeof *pModules);

    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        const GfxShaderStage* pStage = &pShader->pStages[i];

        VkShaderModuleCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = pStage->codeSize,
            .pCode = pStage->pCode,
        };
        VK_CHECK(vkCreateShaderModule(gfxDevice.device, &ci, NULL, &pModules[i]));

        pStages[i] = (VkPipelineShaderStageCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = pStage->stage,
            .module = pModules[i],
            .pName = "main",
            .pSpecializationInfo = pStage->pSpecializationInfo,
        };
    }

    // Groups are laid out as they will be in the shader binding table: ray
    // generation, miss, hit and callable. A shader has at most one group per
    // stage.
    VkRayTracingShaderGroupCreateInfoKHR* pGroups = allocZeroed(pShader->stageCount, sizeof *pGroups);
    uint32_t groupCount = 0;
    uint32_t raygenCount = 0;
    uint32_t missCount = 0;
    uint32_t hitCount = 0;
    uint32_t callableCount = 0;

    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        if (pShader->pStages[i].stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR) {
            pGroups[groupCount++] = generalShaderGroup(i);
            raygenCount++;
        }
    }

    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        if (pShader->pStages[i].stage == VK_SHADER_STAGE_MISS_BIT_KHR) {
            pGroups[groupCount++] = generalShaderGroup(i);
            missCount++;
        }
    }

    // Consecutive hit stages form a group, until a kind of stage repeats
    VkRayTracingShaderGroupCreateInfoKHR* pHitGroup = NULL;
    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        VkShaderStageFlagBits stage = pShader->pStages[i].stage;

        uint32_t* pSlot = NULL;
        if (pHitGroup) {
            switch (stage) {
            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
                pSlot = &pHitGroup->closestHitShader;
                break;
            case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
                pSlot = &pHitGroup->anyHitShader;
                break;
            case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
                pSlot = &pHitGroup->intersectionShader;
                break;
            default:
                // Any other stage ends the group
                pHitGroup = NULL;
                continue;
            }
        } else if (stage != VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR && stage != VK_SHADER_STAGE_ANY_HIT_BIT_KHR &&
                   stage != VK_SHADER_STAGE_INTERSECTION_BIT_KHR) {
            continue;
        }

        if (!pHitGroup || *pSlot != VK_SHADER_UNUSED_KHR) {
            pHitGroup = &pGroups[groupCount++];
            *pHitGroup = generalShaderGroup(VK_SHADER_UNUSED_KHR);
            pHitGroup->type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            hitCount++;
        }

        switch (stage) {
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            pHitGroup->closestHitShader = i;
            break;
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
            pHitGroup->anyHitShader = i;
            break;
        default:
            pHitGroup->intersectionShader = i;
            pHitGroup->type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
            break;
        }
    }

    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        if (pShader->pStages[i].stage == VK_SHADER_STAGE_CALLABLE_BIT_KHR) {
            pGroups[groupCount++] = generalShaderGroup(i);
            callableCount++;
        }
    }

    if (raygenCount != 1) {
        GFX_ERROR("%s has %" PRIu32 " ray generation stages, a ray tracing pipeline needs exactly one",
                  shaderName(pShader), raygenCount);
        goto done;
    }

    VkRayTracingPipelineCreateInfoKHR ci = {
        .sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
        .stageCount = pShader->stageCount,
        .pStages = pStages,
        .groupCount = groupCount,
        .pGroups = pGroups,
        .maxPipelineRayRecursionDepth = pPipeline->maxRecursionDepth,
        .layout = pShader->pipelineLayout,
    };
    VK_CHECK(gfxDevice.fn.vkCreateRayTracingPipelinesKHR(gfxDevice.device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &ci, NULL,
                                                         &pPipeline->pipeline));
    gfxSetDebugName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)pPipeline->pipeline, shaderName(pShader));

    // Shader binding table. Every region starts at the base alignment, and
    // records within a region are a stride apart.
    uint32_t handleSize = pProperties->shaderGroupHandleSize;
    VkDeviceSize stride = gfxAlignTo(handleSize, pProperties->shaderGroupHandleAlignment);
    VkDeviceSize baseAlignment = pProperties->shaderGroupBaseAlignment;

    VkDeviceSize missOffset = gfxAlignTo(stride * raygenCount, baseAlignment);
    VkDeviceSize hitOffset = gfxAlignTo(missOffset + stride * missCount, baseAlignment);
    VkDeviceSize callableOffset = gfxAlignTo(hitOffset + stride * hitCount, baseAlignment);
    VkDeviceSize tableSize = callableOffset + stride * callableCount;

    uint8_t* pHandles = GFX_MALLOC((size_t)groupCount * handleSize);
    VK_CHECK(gfxDevice.fn.vkGetRayTracingShaderGroupHandlesKHR(gfxDevice.device, pPipeline->pipeline, 0, groupCount,
                                                               (size_t)groupCount * handleSize, pHandles));

    gfxCreateBuffer(tableSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &pPipeline->shaderBindingTable);

    uint8_t* pTable = pPipeline->shaderBindingTable.pHostMap;
    memset(pTable, 0, tableSize);

    // The groups were created in table order
    VkDeviceSize regionOffsets[] = {0, missOffset, hitOffset, callableOffset};
    uint32_t regionCounts[] = {raygenCount, missCount, hitCount, callableCount};
    uint32_t group = 0;
    for (uint32_t region = 0; region < GFX_ARRAY_LEN(regionOffsets); region++) {
        for (uint32_t i = 0; i < regionCounts[region]; i++, group++) {
            memcpy(pTable + regionOffsets[region] + i * stride, pHandles + (size_t)group * handleSize, handleSize);
        }
    }

    GFX_FREE(pHandles);

    VkDeviceAddress address = gfxGetBufferDeviceAddress(&pPipeline->shaderBindingTable);
    pPipeline->raygenRegion = shaderBindingTableRegion(address, stride, raygenCount);
    pPipeline->missRegion = shaderBindingTableRegion(address + missOffset, stride, missCount);
    pPipeline->hitRegion = shaderBindingTableRegion(address + hitOffset, stride, hitCount);
    pPipeline->callableRegion = shaderBindingTableRegion(address + callableOffset, stride, callableCount);

    GFX_INFO("Created ray tracing pipeline: %s", shaderName(pShader));
    GFX_DEBUG(" * %" PRIu32 " miss, %" PRIu32 " hit and %" PRIu32 " callable groups", missCount, hitCount,
              callableCount);

done:
    for (uint32_t i = 0; i < pShader->stageCount; i++) {
        vkDestroyShaderModule(gfxDevice.device, pModules[i], NULL);
    }
    GFX_FREE(pGroups);
    GFX_FREE(pModules);
    GFX_FREE(pStages);
}

static void destroyRayTracingPipeline(GfxRayTracingPipeline* pPipeline)
{
    vkDeviceWaitIdle(gfxDevice.device);

    vkDestroyPipeline(gfxDevice.device, pPipeline->pipeline, NULL);
    pPipeline->pipeline = VK_NULL_HANDLE;

    if (pPipeline->shaderBindingTable.buffer) {
        gfxDestroyBuffer(&pPipeline->shaderBindingTable);
    }
}

void gfxCreateRayTracingPipeline(GfxShader* pShader, uint32_t maxRecursionDepth, GfxRayTracingPipeline* pPipeline)
{
    GFX_RESET(pPipeline);

    if (!gfxDevice.rayTracingPipelines) {
        GFX_ERROR("Ray tracing pipelines need VK_KHR_ray_tracing_pipeline to be enabled");
        return;
    }

    if (!(pShader->stages & GFX_RAY_TRACING_STAGES)) {
        GFX_ERROR("%s has no ray tracing stages", shaderName(pShader));
        return;
    }

    pPipeline->pShader = pShader;
    pPipeline->shaderVersion = pShader->version;
    pPipeline->maxRecursionDepth = maxRecursionDepth;

    createRayTracingPipeline(pPipeline);
}

void gfxDestroyRayTracingPipeline(GfxRayTracingPipeline* pPipeline)
{
    destroyRayTracingPipeline(pPipeline);
    GFX_RESET(pPipeline);
}

void gfxCmdBindRayTracingPipeline(VkCommandBuffer cmd, GfxRayTracingPipeline* pPipeline)
{
    // The reload waited for the device, so nothing still uses the old pipeline
    if (pPipeline->shaderVersion != pPipeline->pShader->version) {
        destroyRayTracingPipeline(pPipeline);
        createRayTracingPipeline(pPipeline);
        pPipeline->shaderVersion = pPipeline->pShader->version;
    }

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pPipeline->pipeline);
}

void gfxCmdTraceRays(VkCommandBuffer cmd, const GfxRayTracingPipeline* pPipeline, uint32_t width, uint32_t height,
                     uint32_t depth)
{
    gfxDevice.fn.vkCmdTraceRaysKHR(cmd, &pPipeline->raygenRegion, &pPipeline->missRegion, &pPipeline->hitRegion,
                                   &pPipeline->callableRegion, width, height, depth);
}

void gfxCmdSetDefaultStates(VkCommandBuffer cmd, const GfxAttachment* pAttachmentSet,
                            uint32_t vertexBindingDescriptionCount,
                            const VkVertexInputBindingDescription2EXT* vertexBindingDescriptions,
                            uint32_t vertexAttributeDescriptionCount,
                            const VkVertexInputAttributeDescription2EXT* vertexAttributeDescriptions)
{
    const GfxImage* pImage = pAttachmentSet ? attachmentSetImage(pAttachmentSet) : NULL;
    if (!pImage) {
        GFX_ERROR("An attachment set with a color or a depth attachment is required");
        return;
    }

    // Viewport and sample count follow the attachments actually being rendered
    // to, which is not necessarily the swapchain
    const VkExtent2D extent = {.width = pImage->width, .height = pImage->height};
    const VkSampleCountFlagBits samples = pImage->samples ? pImage->samples : VK_SAMPLE_COUNT_1_BIT;

    const VkViewport viewport = {
        .width = (float)extent.width,
        .height = (float)extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    const VkRect2D scissor = {.extent = extent};
    vkCmdSetViewportWithCount(cmd, 1, &viewport);
    vkCmdSetScissorWithCount(cmd, 1, &scissor);
    vkCmdSetRasterizerDiscardEnable(cmd, VK_FALSE);

    gfxDevice.fn.vkCmdSetVertexInputEXT(cmd, vertexBindingDescriptionCount, vertexBindingDescriptions,
                                        vertexAttributeDescriptionCount, vertexAttributeDescriptions);
    vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    vkCmdSetPrimitiveRestartEnable(cmd, VK_FALSE);

    const VkSampleMask sampleMask = ~0u;
    gfxDevice.fn.vkCmdSetRasterizationSamplesEXT(cmd, samples);
    gfxDevice.fn.vkCmdSetSampleMaskEXT(cmd, samples, &sampleMask);
    gfxDevice.fn.vkCmdSetAlphaToCoverageEnableEXT(cmd, VK_FALSE);
    gfxDevice.fn.vkCmdSetPolygonModeEXT(cmd, VK_POLYGON_MODE_FILL);
    vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
    vkCmdSetFrontFace(cmd, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    vkCmdSetDepthTestEnable(cmd, VK_TRUE);
    vkCmdSetDepthWriteEnable(cmd, VK_TRUE);
    vkCmdSetDepthCompareOp(cmd, GFX_DEPTH_COMPARE_OP);
    vkCmdSetDepthBoundsTestEnable(cmd, VK_FALSE);
    vkCmdSetDepthBiasEnable(cmd, VK_FALSE);
    vkCmdSetStencilTestEnable(cmd, VK_FALSE);

    const VkBool32 colorBlendEnable = VK_FALSE;
    const VkColorBlendEquationEXT colorBlendEquation = {
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };
    const VkColorComponentFlags colorComponents =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    gfxDevice.fn.vkCmdSetLogicOpEnableEXT(cmd, VK_FALSE);
    for (uint32_t i = 0; i < pAttachmentSet->colorAttachmentCount; i++) {
        gfxDevice.fn.vkCmdSetColorBlendEnableEXT(cmd, i, 1, &colorBlendEnable);
        gfxDevice.fn.vkCmdSetColorBlendEquationEXT(cmd, i, 1, &colorBlendEquation);
        gfxDevice.fn.vkCmdSetColorWriteMaskEXT(cmd, i, 1, &colorComponents);
    }
}


// Debug names, labels and GPU timers //

void gfxSetDebugName(VkObjectType type, uint64_t handle, const char* pName)
{
    if (!gfxDevice.fn.vkSetDebugUtilsObjectNameEXT || !handle) {
        return;
    }

    VkDebugUtilsObjectNameInfoEXT info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = pName,
    };

    VK_CHECK(gfxDevice.fn.vkSetDebugUtilsObjectNameEXT(gfxDevice.device, &info));
}

void gfxSetBufferName(const GfxBuffer* pBuffer, const char* pName)
{
    gfxSetDebugName(VK_OBJECT_TYPE_BUFFER, (uint64_t)pBuffer->buffer, pName);

    // Memory shared with other resources keeps the name of its block
    if (!findMemoryBlock(pBuffer->memory)) {
        gfxSetDebugName(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)pBuffer->memory, pName);
    }
}

void gfxSetImageName(const GfxImage* pImage, const char* pName)
{
    gfxSetDebugName(VK_OBJECT_TYPE_IMAGE, (uint64_t)pImage->image, pName);
    gfxSetDebugName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)pImage->imageView, pName);

    if (!findMemoryBlock(pImage->memory)) {
        gfxSetDebugName(VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)pImage->memory, pName);
    }
}

void gfxCmdBeginLabel(VkCommandBuffer cmd, const char* pName)
{
    if (!gfxDevice.fn.vkCmdBeginDebugUtilsLabelEXT) {
        return;
    }

    VkDebugUtilsLabelEXT label = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        .pLabelName = pName,
    };

    gfxDevice.fn.vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
}

void gfxCmdEndLabel(VkCommandBuffer cmd)
{
    if (gfxDevice.fn.vkCmdEndDebugUtilsLabelEXT) {
        gfxDevice.fn.vkCmdEndDebugUtilsLabelEXT(cmd);
    }
}

// Timestamps are written when all earlier commands have finished, so a timer
// measures its commands from when the ones before them are done
void gfxCmdBeginTimer(VkCommandBuffer cmd, const char* pName)
{
    if (!gfxTimers.pFrames) {
        return;
    }

    if (gfxTimers.depth == GFX_MAX_TIMERS) {
        GFX_ERROR("GPU timers are nested deeper than GFX_MAX_TIMERS");
        return;
    }

    GfxTimerFrame* pFrame = &gfxTimers.pFrames[gfxSwapchain.inFlightIndex];

    uint32_t index = UINT32_MAX;
    if (pFrame->count < GFX_MAX_TIMERS) {
        index = pFrame->count++;
        GfxTimerResult* pTimer = &pFrame->timers[index];
        snprintf(pTimer->name, sizeof pTimer->name, "%s", pName);
        pTimer->depth = gfxTimers.depth;
        pTimer->milliseconds = 0.0;
        vkCmdWriteTimestamp2(cmd, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, pFrame->queryPool, 2 * index);
    } else if (!gfxTimers.reported) {
        GFX_WARNING("A frame has more than GFX_MAX_TIMERS GPU timers, the ones from %s on are not measured", pName);
        gfxTimers.reported = true;
    }

    gfxTimers.stack[gfxTimers.depth++] = index;
}

void gfxCmdEndTimer(VkCommandBuffer cmd)
{
    if (!gfxTimers.pFrames) {
        return;
    }

    if (!gfxTimers.depth) {
        GFX_ERROR("gfxCmdEndTimer() called without a timer to end");
        return;
    }

    uint32_t index = gfxTimers.stack[--gfxTimers.depth];
    if (index != UINT32_MAX) {
        GfxTimerFrame* pFrame = &gfxTimers.pFrames[gfxSwapchain.inFlightIndex];
        vkCmdWriteTimestamp2(cmd, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, pFrame->queryPool, 2 * index + 1);
    }
}

uint32_t gfxGetTimerResults(const GfxTimerResult** ppResults)
{
    *ppResults = gfxTimers.results;
    return gfxTimers.resultCount;
}

// Render graph //

void gfxCreateRenderGraph(GfxRenderGraph* pGraph)
{
    GFX_RESET(pGraph);
    pGraph->output = UINT32_MAX;
}

static void destroyGraphPassAttachments(GfxGraphPass* pPass)
{
    if (pPass->pColorImages) {
        gfxDestroyAttachment(&pPass->attachment);
        GFX_FREE(pPass->pColorImages);
        pPass->pColorImages = NULL;
    }
}

void gfxDestroyRenderGraph(GfxRenderGraph* pGraph)
{
    for (uint32_t i = 0; i < pGraph->passCount; i++) {
        GfxGraphPass* pPass = &pGraph->pPasses[i];
        destroyGraphPassAttachments(pPass);
        GFX_FREE(pPass->pName);
        for (uint32_t j = 0; j < pPass->inputCount; j++) {
            GFX_FREE(pPass->inputNames[j]);
        }
        for (uint32_t j = 0; j < pPass->outputCount; j++) {
            GFX_FREE(pPass->outputNames[j]);
        }
    }
    GFX_FREE(pGraph->pPasses);

    for (uint32_t i = 0; i < pGraph->resourceCount; i++) {
        GfxGraphResource* pResource = pGraph->ppResources[i];
        if (pResource->image.image) {
            gfxDestroyImage(&pResource->image);
        }
        if (pResource->multisampled.image) {
            gfxDestroyImage(&pResource->multisampled);
        }
        GFX_FREE(pResource->pName);
        GFX_FREE(pResource);
    }
    GFX_FREE(pGraph->ppResources);
    GFX_FREE(pGraph->pOrder);

    GFX_RESET(pGraph);
}

static uint32_t findGraphResource(const GfxRenderGraph* pGraph, const char* pName)
{
    for (uint32_t i = 0; i < pGraph->resourceCount; i++) {
        if (!strcmp(pGraph->ppResources[i]->pName, pName)) {
            return i;
        }
    }
    return UINT32_MAX;
}

static GfxGraphResource* addGraphResource(GfxRenderGraph* pGraph, const char* pName)
{
    if (findGraphResource(pGraph, pName) != UINT32_MAX) {
        GFX_ERROR("The render graph already has a resource named '%s'", pName);
        return NULL;
    }

    pGraph->ppResources = GFX_REALLOC(pGraph->ppResources, (pGraph->resourceCount + 1) * sizeof *pGraph->ppResources);
    if (!pGraph->ppResources) {
        GFX_ERROR("Unable to allocate memory");
    }

    GfxGraphResource* pResource = allocZeroed(1, sizeof *pResource);
    pResource->pName = copyString(pName, SIZE_MAX);
    pGraph->ppResources[pGraph->resourceCount++] = pResource;
    pGraph->compiled = false;
    return pResource;
}

// Create, or recreate, the image of a resource at its size or the swapchain's
static VkSampleCountFlagBits graphImageSamples(const GfxGraphResource* pResource)
{
    return pResource->desc.samples ? pResource->desc.samples : VK_SAMPLE_COUNT_1_BIT;
}

// A multisampled color image is rendered to on the side and resolved into the
// image itself, while a multisampled depth image is rendered to directly
static bool isResolvedImage(const GfxGraphResource* pResource)
{
    return !pResource->pBuffer && graphImageSamples(pResource) != VK_SAMPLE_COUNT_1_BIT &&
           !isDepthFormat(pResource->desc.format);
}

static void createGraphImage(GfxGraphResource* pResource)
{
    if (pResource->image.image) {
        gfxDestroyImage(&pResource->image);
    }
    if (pResource->multisampled.image) {
        gfxDestroyImage(&pResource->multisampled);
    }

    VkExtent3D extent = {
        .width = pResource->desc.width ? pResource->desc.width : gfxSwapchain.extent.width,
        .height = pResource->desc.height ? pResource->desc.height : gfxSwapchain.extent.height,
        .depth = 1,
    };

    VkFormat format = pResource->desc.format;
    VkImageAspectFlags aspect = isDepthFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    // Only the multisampled image of a resolved image has more than one sample
    VkSampleCountFlagBits samples = isResolvedImage(pResource) ? VK_SAMPLE_COUNT_1_BIT : graphImageSamples(pResource);
    gfxCreateImage(extent, 1, 1, samples, format, VK_IMAGE_TILING_OPTIMAL, pResource->desc.usage, 0, VK_IMAGE_TYPE_2D,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pResource->image);
    gfxCreateImageView(&pResource->image, aspect, VK_IMAGE_VIEW_TYPE_2D);

    // It is never read, only rendered to and resolved
    if (isResolvedImage(pResource)) {
        gfxCreateImage(extent, 1, 1, graphImageSamples(pResource), format, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, VK_IMAGE_TYPE_2D, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       &pResource->multisampled);
        gfxCreateImageView(&pResource->multisampled, aspect, VK_IMAGE_VIEW_TYPE_2D);

        char name[256];
        snprintf(name, sizeof name, "%s (multisampled)", pResource->pName);
        gfxSetImageName(&pResource->multisampled, name);
    }

    gfxSetImageName(&pResource->image, pResource->pName);

    pResource->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    pResource->written = false;
}

GfxImage* gfxAddGraphImage(GfxRenderGraph* pGraph, const GfxGraphImageDesc* pDesc)
{
    if ((!pDesc->width || !pDesc->height) && !gfxSwapchain.swapchain) {
        GFX_ERROR("Image '%s' follows the swapchain size, so the swapchain has to be created first", pDesc->pName);
        return NULL;
    }

    if (pDesc->samples > VK_SAMPLE_COUNT_1_BIT) {
        const VkPhysicalDeviceLimits* pLimits = &gfxDevice.properties.physicalDevice.limits;
        VkSampleCountFlags supported = isDepthFormat(pDesc->format) ? pLimits->framebufferDepthSampleCounts
                                                                    : pLimits->framebufferColorSampleCounts;
        if (!(supported & pDesc->samples)) {
            GFX_ERROR("Image '%s' asks for %d samples, which the device does not support for it", pDesc->pName,
                      (int)pDesc->samples);
            return NULL;
        }

        if (!isDepthFormat(pDesc->format) && !(pDesc->usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
            GFX_ERROR("Multisampled image '%s' is resolved into as an attachment, so it needs "
                      "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT",
                      pDesc->pName);
            return NULL;
        }
    }

    GfxGraphResource* pResource = addGraphResource(pGraph, pDesc->pName);
    if (!pResource) {
        return NULL;
    }

    pResource->desc = *pDesc;
    pResource->desc.pName = pResource->pName;
    createGraphImage(pResource);

    return &pResource->image;
}

void gfxAddGraphBuffer(GfxRenderGraph* pGraph, const char* pName, const GfxBuffer* pBuffer)
{
    GfxGraphResource* pResource = addGraphResource(pGraph, pName);
    if (pResource) {
        pResource->pBuffer = pBuffer;
    }
}

void gfxAddGraphPass(GfxRenderGraph* pGraph, const GfxPassDesc* pDesc)
{
    pGraph->pPasses = GFX_REALLOC(pGraph->pPasses, (pGraph->passCount + 1) * sizeof *pGraph->pPasses);
    if (!pGraph->pPasses) {
        GFX_ERROR("Unable to allocate memory");
    }

    GfxGraphPass* pPass = &pGraph->pPasses[pGraph->passCount++];
    *pPass = (GfxGraphPass){
        .pName = copyString(pDesc->pName ? pDesc->pName : "(unnamed)", SIZE_MAX),
        .type = pDesc->type,
        .function = pDesc->function,
        .pUserData = pDesc->pUserData,
    };

    // The lists end at the first name that is not set
    while (pPass->inputCount < GFX_MAX_PASS_RESOURCES && pDesc->inputs[pPass->inputCount]) {
        pPass->inputNames[pPass->inputCount] = copyString(pDesc->inputs[pPass->inputCount], SIZE_MAX);
        pPass->inputCount++;
    }
    while (pPass->outputCount < GFX_MAX_PASS_RESOURCES && pDesc->outputs[pPass->outputCount]) {
        pPass->outputNames[pPass->outputCount] = copyString(pDesc->outputs[pPass->outputCount], SIZE_MAX);
        pPass->outputCount++;
    }

    pGraph->compiled = false;
}

void gfxSetGraphOutput(GfxRenderGraph* pGraph, const char* pName)
{
    uint32_t index = findGraphResource(pGraph, pName);
    if (index == UINT32_MAX || pGraph->ppResources[index]->pBuffer) {
        GFX_ERROR("The render graph has no image named '%s'", pName);
        return;
    }
    pGraph->output = index;
}

GfxImage* gfxGetGraphImage(const GfxRenderGraph* pGraph, const char* pName)
{
    uint32_t index = findGraphResource(pGraph, pName);
    if (index == UINT32_MAX || pGraph->ppResources[index]->pBuffer) {
        return NULL;
    }
    return &pGraph->ppResources[index]->image;
}

static bool isAttachmentImage(const GfxGraphResource* pResource)
{
    return !pResource->pBuffer && (pResource->desc.usage &
                                   (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
}

// Gather the attachments of the graphics passes from the images they write.
// The attachment set refers to copies of the images, so it is rebuilt when the
// images are recreated.
static bool buildGraphPassAttachments(GfxRenderGraph* pGraph)
{
    for (uint32_t i = 0; i < pGraph->passCount; i++) {
        GfxGraphPass* pPass = &pGraph->pPasses[i];
        destroyGraphPassAttachments(pPass);

        if (pPass->type != GFX_PASS_GRAPHICS) {
            continue;
        }

        pPass->pColorImages = allocZeroed(GFX_MAX_PASS_RESOURCES, sizeof *pPass->pColorImages);
        pPass->hasDepth = false;
        uint32_t colorCount = 0;
        VkSampleCountFlagBits samples = 0;

        for (uint32_t j = 0; j < pPass->outputCount; j++) {
            const GfxGraphResource* pResource = pGraph->ppResources[pPass->outputs[j]];
            if (!isAttachmentImage(pResource)) {
                continue;
            }

            if (samples && graphImageSamples(pResource) != samples) {
                GFX_ERROR("The attachments of pass '%s' have different sample counts, '%s' has %d", pPass->pName,
                          pResource->pName, (int)graphImageSamples(pResource));
                return false;
            }
            samples = graphImageSamples(pResource);

            if (isDepthFormat(pResource->desc.format)) {
                if (pPass->hasDepth) {
                    GFX_ERROR("Pass '%s' writes more than one depth image", pPass->pName);
                    return false;
                }
                pPass->depthImage = pResource->image;
                pPass->hasDepth = true;
            } else {
                // A multisampled image is rendered to on the side
                pPass->pColorImages[colorCount++] =
                    isResolvedImage(pResource) ? pResource->multisampled : pResource->image;
            }
        }

        // A pass writing only depth, such as a shadow map pass, is fine
        if (!colorCount && !pPass->hasDepth) {
            GFX_ERROR("Graphics pass '%s' writes no color or depth attachment", pPass->pName);
            return false;
        }

        gfxCreateAttachment(colorCount, pPass->pColorImages, pPass->hasDepth ? &pPass->depthImage : NULL, NULL,
                            &pPass->attachment);

        // Every multisampled color attachment resolves into its own image. The
        // multisampled contents are not needed after that, so they are not
        // stored.
        uint32_t color = 0;
        for (uint32_t j = 0; j < pPass->outputCount; j++) {
            const GfxGraphResource* pResource = pGraph->ppResources[pPass->outputs[j]];
            if (!isAttachmentImage(pResource) || isDepthFormat(pResource->desc.format)) {
                continue;
            }

            VkRenderingAttachmentInfo* pInfo = &pPass->attachment.pRenderingAttachmentInfos[color++];
            if (isResolvedImage(pResource)) {
                pInfo->resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
                pInfo->resolveImageView = pResource->image.imageView;
                pInfo->resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                pInfo->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }
        }
    }

    return true;
}

// Order the passes after the passes that write what they read, with a depth
// first search. Returns false on a cycle.
static bool visitGraphPass(GfxRenderGraph* pGraph, const uint32_t* pWriters, uint8_t* pState, uint32_t pass,
                           uint32_t* pOrderCount)
{
    // 0 is unvisited, 1 is on the current path and 2 is done
    if (pState[pass] == 2) {
        return true;
    }
    if (pState[pass] == 1) {
        GFX_ERROR("The render graph has a cycle through pass '%s'", pGraph->pPasses[pass].pName);
        return false;
    }

    pState[pass] = 1;
    const GfxGraphPass* pPass = &pGraph->pPasses[pass];
    for (uint32_t i = 0; i < pPass->inputCount; i++) {
        uint32_t writer = pWriters[pPass->inputs[i]];
        if (writer != UINT32_MAX && writer != pass && !visitGraphPass(pGraph, pWriters, pState, writer, pOrderCount)) {
            return false;
        }
    }
    pState[pass] = 2;

    pGraph->pOrder[(*pOrderCount)++] = pass;
    return true;
}

static bool compileRenderGraph(GfxRenderGraph* pGraph)
{
    bool ok = true;

    // Resolve the names, and find the one pass writing each resource
    uint32_t* pWriters = GFX_MALLOC(GFX_MAX(pGraph->resourceCount, 1) * sizeof *pWriters);
    for (uint32_t i = 0; i < pGraph->resourceCount; i++) {
        pWriters[i] = UINT32_MAX;
    }

    for (uint32_t i = 0; i < pGraph->passCount && ok; i++) {
        GfxGraphPass* pPass = &pGraph->pPasses[i];

        for (uint32_t j = 0; j < pPass->inputCount && ok; j++) {
            pPass->inputs[j] = findGraphResource(pGraph, pPass->inputNames[j]);
            if (pPass->inputs[j] == UINT32_MAX) {
                GFX_ERROR("Pass '%s' reads '%s', which is not in the render graph", pPass->pName, pPass->inputNames[j]);
                ok = false;
            }
        }

        for (uint32_t j = 0; j < pPass->outputCount && ok; j++) {
            uint32_t resource = findGraphResource(pGraph, pPass->outputNames[j]);
            pPass->outputs[j] = resource;
            if (resource == UINT32_MAX) {
                GFX_ERROR("Pass '%s' writes '%s', which is not in the render graph", pPass->pName,
                          pPass->outputNames[j]);
                ok = false;
            } else if (pWriters[resource] != UINT32_MAX) {
                GFX_ERROR("'%s' is written by both pass '%s' and pass '%s', but a render graph resource can only be "
                          "written by one pass",
                          pPass->outputNames[j], pGraph->pPasses[pWriters[resource]].pName, pPass->pName);
                ok = false;
            } else {
                pWriters[resource] = i;
            }
        }
    }

    if (ok) {
        for (uint32_t i = 0; i < pGraph->passCount; i++) {
            const GfxGraphPass* pPass = &pGraph->pPasses[i];
            for (uint32_t j = 0; j < pPass->inputCount; j++) {
                if (pWriters[pPass->inputs[j]] == UINT32_MAX) {
                    GFX_WARNING("Pass '%s' reads '%s', which no pass writes", pPass->pName, pPass->inputNames[j]);
                }
            }
        }

        GFX_FREE(pGraph->pOrder);
        pGraph->pOrder = GFX_MALLOC(GFX_MAX(pGraph->passCount, 1) * sizeof *pGraph->pOrder);
        uint8_t* pState = allocZeroed(GFX_MAX(pGraph->passCount, 1), sizeof *pState);
        uint32_t orderCount = 0;
        for (uint32_t i = 0; i < pGraph->passCount && ok; i++) {
            ok = visitGraphPass(pGraph, pWriters, pState, i, &orderCount);
        }
        GFX_FREE(pState);
    }

    GFX_FREE(pWriters);

    if (ok) {
        ok = buildGraphPassAttachments(pGraph);
    }

    if (ok) {
        GFX_DEBUG("Render graph execution order:");
        for (uint32_t i = 0; i < pGraph->passCount; i++) {
            GFX_DEBUG(" %" PRIu32 ". %s", i + 1, pGraph->pPasses[pGraph->pOrder[i]].pName);
        }
    }

    pGraph->compiled = ok;
    return ok;
}

// Recreate the images that follow the swapchain size if it has changed
static void resizeGraphImages(GfxRenderGraph* pGraph)
{
    bool resized = false;

    for (uint32_t i = 0; i < pGraph->resourceCount; i++) {
        GfxGraphResource* pResource = pGraph->ppResources[i];
        if (pResource->pBuffer || (pResource->desc.width && pResource->desc.height)) {
            continue;
        }

        if (pResource->image.width != gfxSwapchain.extent.width ||
            pResource->image.height != gfxSwapchain.extent.height) {
            createGraphImage(pResource);
            resized = true;
        }
    }

    if (resized) {
        buildGraphPassAttachments(pGraph);
    }
}

// How an image in a layout is accessed, which a barrier to or from it has to
// synchronize with
static void layoutAccess(VkImageLayout layout, VkPipelineStageFlags2* pStage, VkAccessFlags2* pAccess)
{
    switch (layout) {
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        *pStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        *pAccess = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        *pStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        *pAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        *pStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        *pAccess = VK_ACCESS_2_SHADER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_GENERAL:
        *pStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        *pAccess = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        *pStage = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
        *pAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
        break;
    default:
        // Nothing to wait for, the contents are discarded
        *pStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        *pAccess = VK_ACCESS_2_NONE;
        break;
    }
}

// Make what was done to a resource before visible to how it is used next. A
// barrier is recorded also when an image stays in its layout, since it is the
// barrier that orders one pass after another.
static void transitionGraphResource(VkCommandBuffer cmd, GfxGraphResource* pResource, VkImageLayout layout, bool write)
{
    if (pResource->pBuffer) {
        gfxMemoryBarrier(cmd, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                         pResource->written ? VK_ACCESS_2_MEMORY_WRITE_BIT : VK_ACCESS_2_NONE,
                         VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                         VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
        pResource->written = write;
        return;
    }

    VkPipelineStageFlags2 srcStage, dstStage;
    VkAccessFlags2 srcAccess, dstAccess;
    layoutAccess(pResource->layout, &srcStage, &srcAccess);
    layoutAccess(layout, &dstStage, &dstAccess);

    VkFormat format = pResource->desc.format;
    VkImageSubresourceRange range = {
        .aspectMask = isDepthFormat(format)
                          ? VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0)
                          : VK_IMAGE_ASPECT_COLOR_BIT,
        .levelCount = 1,
        .layerCount = 1,
    };

    gfxImageBarrier(cmd, srcStage, srcAccess, dstStage, dstAccess, pResource->layout, layout, pResource->image.image,
                    &range);
    pResource->layout = layout;
    pResource->written = write;
}

// Layout a pass reads an image in
static VkImageLayout graphReadLayout(const GfxGraphResource* pResource)
{
    if (pResource->desc.usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    if (pResource->desc.usage & VK_IMAGE_USAGE_STORAGE_BIT) {
        return VK_IMAGE_LAYOUT_GENERAL;
    }
    return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
}

// Layout a pass writes an image in
static VkImageLayout graphWriteLayout(const GfxGraphPass* pPass, const GfxGraphResource* pResource)
{
    if (pPass->type == GFX_PASS_GRAPHICS && isAttachmentImage(pResource)) {
        return isDepthFormat(pResource->desc.format) ? depthAttachmentLayout(pResource->desc.format)
                                                     : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    return VK_IMAGE_LAYOUT_GENERAL;
}

void gfxCmdExecuteRenderGraph(VkCommandBuffer cmd, GfxRenderGraph* pGraph)
{
    if (!pGraph->compiled && !compileRenderGraph(pGraph)) {
        return;
    }

    resizeGraphImages(pGraph);

    for (uint32_t i = 0; i < pGraph->passCount; i++) {
        GfxGraphPass* pPass = &pGraph->pPasses[pGraph->pOrder[i]];

        // A pass is labeled and timed with its barriers included
        gfxCmdBeginLabel(cmd, pPass->pName);
        gfxCmdBeginTimer(cmd, pPass->pName);

        for (uint32_t j = 0; j < pPass->inputCount; j++) {
            GfxGraphResource* pResource = pGraph->ppResources[pPass->inputs[j]];
            transitionGraphResource(cmd, pResource, graphReadLayout(pResource), false);
        }

        for (uint32_t j = 0; j < pPass->outputCount; j++) {
            GfxGraphResource* pResource = pGraph->ppResources[pPass->outputs[j]];
            transitionGraphResource(cmd, pResource, graphWriteLayout(pPass, pResource), true);

            // The image itself is written by the resolve, as an attachment. The
            // multisampled image is cleared and rendered from scratch, so its
            // contents are discarded, but the previous frame's rendering to it
            // has to finish first.
            if (pPass->type == GFX_PASS_GRAPHICS && isResolvedImage(pResource)) {
                gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                pResource->multisampled.image, NULL);
            }
        }

        if (pPass->type == GFX_PASS_GRAPHICS) {
            // Every color attachment is cleared to the color of its image
            uint32_t color = 0;
            for (uint32_t j = 0; j < pPass->outputCount; j++) {
                const GfxGraphResource* pResource = pGraph->ppResources[pPass->outputs[j]];
                if (isAttachmentImage(pResource) && !isDepthFormat(pResource->desc.format)) {
                    VkRenderingAttachmentInfo* pInfo = &pPass->attachment.pRenderingAttachmentInfos[color++];
                    pInfo->clearValue.color = pResource->desc.clearColor;
                    pInfo->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                }
            }

            beginRendering(cmd, &pPass->attachment);
            pPass->function(cmd, &pPass->attachment, pPass->pUserData);
            vkCmdEndRendering(cmd);
        } else {
            pPass->function(cmd, NULL, pPass->pUserData);
        }

        gfxCmdEndTimer(cmd);
        gfxCmdEndLabel(cmd);
    }

    // Hand the output over ready to be presented
    if (pGraph->output != UINT32_MAX) {
        transitionGraphResource(cmd, pGraph->ppResources[pGraph->output], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, false);
    }
}

#endif

#ifdef __cplusplus
}
#endif
