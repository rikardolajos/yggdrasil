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

#include <vulkan/vulkan.h>

#include <assert.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define YG_ARRAY_LEN(x) (uint32_t)(sizeof(x) / sizeof *(x))

#ifndef YG_MALLOC
#define YG_MALLOC(sz) ygCheckedMalloc(sz)
#endif

#ifndef YG_REALLOC
#define YG_REALLOC(p, sz) realloc(p, sz)
#endif

#ifndef YG_FREE
#define YG_FREE(p) free(p)
#endif

extern YgDevice ygDevice;

struct VulkanErrors {
    VkResult result;
    const char* string;
} ygVulkanErrors[] = {
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

#define YG_INFO(fmt, ...) fprintf(stdout, "INFO: " fmt "\n", ##__VA_ARGS__);

#ifdef NDEBUG
#define YG_DEBUG(fmt, ...)
#else
#define YG_DEBUG(fmt, ...)                                                     \
    fprintf(stdout, "\x1B[1;92mDEBUG: \x1B[0m" fmt "\n", ##__VA_ARGS__);
#endif

#define YG_WARNING(fmt, ...)                                                   \
    fprintf(stderr, "\x1B[1;93mWARNING: \x1B[0m%s:%d: " fmt "\n", __FILE__,    \
            __LINE__, ##__VA_ARGS__);

#define YG_ERROR(fmt, ...)                                                     \
    fprintf(stderr, "\x1B[1;91mERROR: \x1B[0m%s:%d: " fmt "\n", __FILE__,      \
            __LINE__, ##__VA_ARGS__);                                          \
    abort();

#define VK_CHECK(res)                                                          \
    do {                                                                       \
        if ((res)) {                                                           \
            for (size_t i = 0; i < YG_ARRAY_LEN(ygVulkanErrors); i++) {        \
                if ((res) == ygVulkanErrors[i].result) {                       \
                    YG_ERROR(ygVulkanErrors[i].string);                        \
                }                                                              \
            }                                                                  \
        }                                                                      \
    } while (0)

typedef struct {
    VkPhysicalDeviceProperties physicalDevice;
    VkPhysicalDeviceMemoryProperties memory;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipeline;
} YgDeviceProperties;

typedef struct {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    YgDeviceProperties properties;
    uint32_t queueFamilyIndex;
    VkCommandPool commandPool;
    VkQueue queue;
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    bool vsync;
} YgDevice;

typedef struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
    VkDeviceSize size;
    void* pHostMap;
} YgBuffer;

typedef struct {
    VkImage image;
} YgImage;

void ygCreateInstance(uint32_t apiVersion, const char** ppInstanceExtensions,
                      uint32_t instanceExtensionCount);

void ygDestroyInstance();

void ygCreateDevice(uint32_t physicalDeviceIndex,
                    const char** ppDeviceExtensions,
                    uint32_t deviceExtensionCount,
                    VkPhysicalDeviceFeatures2* features, VkSurfaceKHR surface);

void ygDestroyDevice();

YgBuffer ygCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties);

void ygDestroyBuffer(YgBuffer* pBuffer);

void ygBufferCopyFromHost(const YgBuffer* pBuffer, const void* pData,
                          VkDeviceSize size, VkDeviceSize offset);

inline void* ygCheckedMalloc(size_t sz)
{
    void* p = malloc(sz);
    if (!p) {
        YG_ERROR("Unable to allocate memory");
    }
    return p;
}

inline VkDeviceSize ygAlignTo(VkDeviceSize value, VkDeviceSize alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

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

inline VkFormat ygFindSupportedFormat(VkFormat* pCandidates,
                                      uint32_t candidateCount,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    for (uint32_t i = 0; i < candidateCount; i++) {
        VkFormat c = *pCandidates[i];

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


inline VkFormat ygFindDepthFormat(ptr<Device> pDevice)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };

    return ygFindSupportedFormat(
        candidates, YG_ARRAY_LEN(candidates), VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

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

#define YGGDRASIL_IMPLEMENTATION
#ifdef YGGDRASIL_IMPLEMENTATION

static YgDevice ygDevice;

#define YG_RESET(x) memset((x), 0, sizeof(*(x)))

#ifndef NDEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL ygDebugCallback(
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

static VkResult ygCreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func =
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void
ygDestroyDebugUtilsMessengerEXT(VkInstance instance,
                                VkDebugUtilsMessengerEXT debugMessenger,
                                const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) {
        func(instance, debugMessenger, pAllocator);
    }
}

static void
ygPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& ci)
{
    ci = (VkDebugUtilsMessengerCreateInfoEXT){
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = ygDebugCallback,
    };
}

void ygCreateDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT ci;
    populateDebugMessengerCreateInfo(ci);

    VK_CHECK(ygCreateDebugUtilsMessengerEXT(ygDevice.instance, &ci, NULL,
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
        .ppExtensions = ppInstanceExtensions,
    };

#ifndef NDEBUG
    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
    ci.enabledLayerCount = YG_ARRAY_LEN(layers);
    ci.ppEnabledLayerNames = layers;
#endif

    VK_CHECK(vkCreateInstance(&ci, NULL, &ygDevice.instance));

    uint32_t version;
    VK_CHECK(vkEnumerateInstanceVersion(&version));
    YG_INFO("Created Vulkan instance: %d.%d.%d", VK_API_VERSION_MAJOR(version),
            VK_API_VERSION_MINOR(version), VK_API_VERSION_PATCH(version));
}

void ygDestroyInstance()
{
    if (ygDevice.instance) {
        vkDestroyInstance(ygDevice.instance, NULL);
    }
}

static bool ygCheckDeviceExtensionSupport(const char** ppDeviceExtensions,
                                          uint32_t deviceExtensionCount)
{
    uint32_t n;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(ygDevice.physicalDevice, NULL,
                                                  &n, NULL));
    VkExtensionProperties* pAvailable = YG_MALLOC(n * sizeof *pAvailable);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(ygDevice.physicalDevice, NULL,
                                                  &n, pAvailable));

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
            YG_ERROR("The requested extension %s is not available", e);
            result = false;
        }
    }

    YG_FREE(pAvailable);

    return result;
}

static uint32_t ygGetQueueFamilyIndex(VkSurfaceKHR surface,
                                      uint32_t requiredFamilyFlags)
{
    uint32_t n;
    vkGetPhysicalDeviceQueueFamilyProperties(ygDevice.physicalDevice, &n, NULL);
    VkQueueFamilyProperties* pProps = YG_MALLOC(n * sizeof *pProps);
    vkGetPhysicalDeviceQueueFamilyProperties(ygDevice.physicalDevice, &n,
                                             queueFamilyProperties.data());

    if (!n) {
        YG_ERROR("No Vulkan queue family available");
    }

    YG_DEBUG("Available queue families for selected device: {}", n);

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
        YG_ERROR("Instance not created");
    }

    if (ygDevice.device) {
        YG_ERROR("Device already created");
    }

    // Iterate all physical devices
    uint32_t n;
    VK_CHECK(vkEnumeratePhysicalDevices(mInstance, &n, NULL));
    VkPhysicalDevice* pPhysicalDevices =
        YG_MALLOC(n * sizeof *pPhysicalDevices);
    VK_CHECK(vkEnumeratePhysicalDevices(mInstance, &n, physicalDevices.data()));

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

        vkGetPhysicalDeviceProperties2(physicalDevices[i], &prop);

        if (i == physicalDeviceIndex) {
            ygDevice.physicalDevce = physicalDevices[i];
            vkGetPhysicalDeviceProperties(ygDevice.physicalDevce,
                                          &ygDevice.properties.physicalDevice);
            vkGetPhysicalDeviceMemoryProperties(ygDevice.physicalDevce,
                                                &ygDevice.properties.memory);
            ygDevice.properties.rayTracingPipeline = rtp;
        }

        YG_INFO(" * %s, driver: %s %s, Vulkan %d.%d.%d %s",
                prop.properties.deviceName, driver.driverName,
                driver.driverInfo,
                VK_API_VERSION_MAJOR(prop.properties.apiVersion),
                VK_API_VERSION_MINOR(prop.properties.apiVersion),
                VK_API_VERSION_PATCH(prop.properties.apiVersion),
                i == physicalDeviceIndex ? "(chosen)" : "");
    }

    YG_FREE(pPhysicalDevices);

    uin32_t queueFamilyIndex = ygGetQueueFamilyIndex(
        ygDevice.physicalDevice, surface,
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
}

YgBuffer ygCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties)
{
    if (!ygDevice.device) {
        YG_ERROR("Device not initialized");
    }

    YgBuffer buffer = {
        .usage = usage,
        .properties = properties,
    };

    VkBufferCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VK_CHECK(vkCreateBuffer(ygDevice.device, &ci, NULL, &buffer.buffer));

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(ygDevice.device, buffer.buffer, &memReqs);

    buffer.size = memReqs.size;

    VkMemoryAllocateFlagsInfo allocFlagInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
    };

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = ygFindMemoryType(ygDevice.device,
                                            memReqs.memoryTypeBits, properties),
    };

    if (buffer.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocInfo.pNext = &allocFlagInfo;
    }

    VK_CHECK(
        vkAllocateMemory(ygDevice.device, &allocInfo, NULL, &buffer.memory));

    VK_CHECK(
        vkBindBufferMemory(ygDevice.device, buffer.buffer, buffer.memory, 0));

    // Map memory if memory is host coherent
    if (buffer.properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        VK_CHECK(vkMapMemory(ygDevice.device, buffer.memory, 0, size, 0,
                             &buffer.pHostMap));
    }

    return buffer;
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

void ygBufferCopyFromHost(const YgBuffer* pBuffer, const void* pData,
                          VkDeviceSize size, VkDeviceSize offset)
{
    // Check if we need a staging buffer or not
    if (!(pBuffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        // Set up staging buffer
        YgBuffer staging =
            ygCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // Copy to staging buffer
        ygBufferCopyFromHost(&staging, pData, size, 0);

        // Transfer from staging buffer to this buffer
        VkCommandBuffer cmd = ygCmdBegin();

        VkBufferCopy region = {
            .dstOffset = offset,
            .size = size,
        };
        vkCmdCopyBuffer(cmd, staging.getBuffer(), mBuffer, 1, &region);

        ygCmdEnd(cmd);

        ygDestroyBuffer(&staging);
    } else {
        // Transfer directly without staging buffer
        char* pOffsettedHostMap = (char*)pBuffer->pHostMap + offset;
        memcpy(pOffsettedHostMap, pData, size);
    }
}


#endif
