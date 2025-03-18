#define YGGDRASIL_IMPLEMENTATION
#include "yggdrasil.h"

#include <GLFW/glfw3.h>

GLFWwindow* pWindow;
VkSurfaceKHR surface;

void createWindow()
{
    if (!glfwInit()) {
        YG_ERROR("Failed to initialize GLFW");
    }

    pWindow = glfwCreateWindow(640, 480, "Yggdrasil Example", NULL, NULL);
    if (!pWindow) {
        glfwTerminate();
        YG_ERROR("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
}

void createInstance()
{
    uint32_t instanceExtensionCount;
    glfwGetRequiredInstanceExtensions(&instanceExtensionCount);
    const char** ppInstanceExtensions =
        glfwGetRequiredInstanceExtensions(&instanceExtensionCount);

    ygCreateInstance(VK_API_VERSION_1_3, ppInstanceExtensions,
                     instanceExtensionCount);
}

void createDevice()
{
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

    ygCreateDevice(0, deviceExtensions, YG_ARRAY_LEN(deviceExtensions),
                   &features, surface);
}

void cleanUp()
{
    ygDestroyDevice();
    ygDestroyInstance();

    glfwTerminate();
}

int main()
{
    createWindow();

    createInstance();

    VK_CHECK(
        glfwCreateWindowSurface(ygDevice.instance, pWindow, NULL, &surface));

    createDevice();

    while (!glfwWindowShouldClose(pWindow)) {
        // if (swapchain.recreated) {
        //     //
        // }

        // VkCommandBuffer cmd = ygAcquireNextImage();

        // Push descriptor

        // Draw

        // ygPresent(cmd, image);

        glfwPollEvents();
    }

    cleanUp();
}