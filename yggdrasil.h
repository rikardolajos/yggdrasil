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

#define YG_ARRAY_LEN(x) (sizeof(x) / sizeof *(x))

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

#define YG_ERROR(str)                                                          \
    fprintf(stderr, "\x1B[1;91mERROR: \x1B[0m %s:%d: %s\n", __FILE__,          \
            __LINE__, str);                                                    \
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
} DeviceProperties;

typedef struct {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    DeviceProperties properties;
    uint32_t queueFamilyIndex;
    VkCommandPool commandPool;
    VkQueue queue;
    bool vsync;
} Device;

typedef struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
    VkDeviceSize size;
    void* pHostMap;
} Buffer;

typedef struct {
    VkImage image;
} Image;


void ygCreateDevice(uint32_t apiVersion, uint32_t physicalDeviceIndex,
                    const char** ppInstanceExtensions,
                    uint32_t instanceExtensionCount,
                    const char** ppDeviceExtensions,
                    uint32_t deviceExtensionCount,
                    VkPhysicalDeviceFeatures2* features);

void ygDestroyDevice();

Buffer ygCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties);

void ygDestroyBuffer(Buffer* pBuffer);

void ygBufferCopyFromHost(Buffer* pBuffer, const void* pData, VkDeviceSize size,
                          VkDeviceSize offset);

#define YGGDRASIL_IMPLEMENTATION
#ifdef YGGDRASIL_IMPLEMENTATION

static Device device;

#define YG_RESET(x) memset((x), 0, sizeof(*(x)))

static void ygCreateInstance(uint32_t apiVersion,
                             const char** ppInstanceExtensions,
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

    VK_CHECK(vkCreateInstance(&ci, NULL, &device.instance));
}

void ygCreateDevice(uint32_t apiVersion, uint32_t physicalDeviceIndex,
                    const char** ppInstanceExtensions,
                    uint32_t instanceExtensionCount,
                    const char** ppDeviceExtensions,
                    uint32_t deviceExtensionCount,
                    VkPhysicalDeviceFeatures2* features)
{
    if (device.device) {
        YG_ERROR("Device already created");
    }

    ygCreateInstance(apiVersion, ppInstanceExtensions, instanceExtensionCount);
}

void ygDestroyDevice(Device* pDevice)
{
    vkDeviceWaitIdle(pDevice->device);

    if (pDevice->commandPool) {
        vkDestroyCommandPool(pDevice->device, pDevice->commandPool, NULL);
    }
    if (pDevice->device) {
        vkDestroyDevice(pDevice->device, NULL);
    }
    if (pDevice->surface) {
        vkDestroySurfaceKHR(pDevice->instance, pDevice->surface, NULL);
    }
    if (pDevice->instance) {
        vkDestroyInstance(pDevice->instance, NULL);
    }

    YG_RESET(pDevice);
}

Buffer ygCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties)
{
    if (!device.device) {
        YG_ERROR("Device not initialized");
    }

    Buffer buffer = {
        .usage = usage,
        .properties = properties,
    };

    VkBufferCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VK_CHECK(vkCreateBuffer(device.device, &ci, NULL, &buffer.buffer));

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device.device, buffer.buffer, &memReqs);

    buffer.size = memReqs.size;

    VkMemoryAllocateFlagsInfo allocFlagInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
    };

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex =
            ygFindMemoryType(device.device, memReqs.memoryTypeBits, properties),
    };

    if (buffer.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocInfo.pNext = &allocFlagInfo;
    }

    VK_CHECK(vkAllocateMemory(device.device, &allocInfo, NULL, &buffer.memory));

    VK_CHECK(
        vkBindBufferMemory(device.device, buffer.buffer, buffer.memory, 0));

    // Map memory if it is host coherent
    if (buffer.properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        VK_CHECK(vkMapMemory(device.device, buffer.memory, 0, size, 0,
                             &buffer.pHostMap));
    }

    return buffer;
}

void ygDestroyBuffer(Buffer* pBuffer)
{
    vkDeviceWaitIdle(device.device);

    if (pBuffer->pHostMap) {
        vkUnmapMemory(device.device, pBuffer->memory);
    }

    vkDestroyBuffer(device.device, pBuffer->buffer, NULL);
    vkFreeMemory(device.device, pBuffer->memory, NULL);

    YG_RESET(pBuffer);
}

void ygBufferCopyFromHost(Buffer* pBuffer, const void* pData, VkDeviceSize size,
                          VkDeviceSize offset)
{
    // Check if we need a staging buffer or not
    if (!(pBuffer->properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        // Set up staging buffer
        Buffer staging =
            ygCreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // Copy to staging buffer
        ygBufferCopyFromHost(&staging, pData, size, 0);

        // Transfer from staging buffer to this buffer
        VkCommandBuffer cmd = Helpers::cmdBegin(mpDevice);

        VkBufferCopy region = {
            .dstOffset = offset,
            .size = size,
        };
        vkCmdCopyBuffer(cmd, staging.getBuffer(), mBuffer, 1, &region);

        Helpers::cmdEnd(mpDevice, cmd);

        ygDestroyBuffer(&staging);
    } else {
        // Transfer directly without staging buffer
        char* pOffsettedHostMap = (char*)pBuffer->pHostMap + offset;
        memcpy(pOffsettedHostMap, pData, size);
    }
}


#endif
