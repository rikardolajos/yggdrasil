#define YGGDRASIL_IMPLEMENTATION
#include "yggdrasil.h"

#include <GLFW/glfw3.h>

void createDevice()
{
    uint32_t instanceExtensionCount;
    glfwGetRequiredInstanceExtensions(&instanceExtensionCount);
    const char** instanceExtensions =
        glfwGetRequiredInstanceExtensions(&instanceExtensionCount);

    VkPhysicalDeviceVulkan12Features vk12Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features vk13Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &vk12Features,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceVulkan14Features vk14Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = &vk13Features,
        .pushDescriptor = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vk14Features,
    };

    const char** deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    ygCreateDevice(VK_API_VERSION_1_3, 0, instanceExtensions,
                   instanceExtensionCount, deviceExtensions,
                   YG_ARRAY_LEN(deviceExtensions), &features);
}

int main()
{
    createDevice();

    ygDestroyDevice();
}