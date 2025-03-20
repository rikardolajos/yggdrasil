#define YGGDRASIL_IMPLEMENTATION
#include "yggdrasil.h"

#include <GLFW/glfw3.h>

GLFWwindow* pWindow;
VkSurfaceKHR surface;

void framebufferSizeCallback(uint32_t* pWidth, uint32_t* pHeight)
{
    *pWidth = 0;
    *pHeight = 0;

    // Handle minimization
    do {
        glfwGetFramebufferSize(pWindow, pWidth, pHeight);
        glfwWaitEvents();
    } while (pWidth == 0 || pHeight == 0);
}

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
    const char** deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

    ygCreateDevice(0, deviceExtensions, YG_ARRAY_LEN(deviceExtensions),
                   &features, surface);
}

void cleanUp()
{
    ygDestroySwapchain();
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

    ygCreateSwapchain(2, framebufferSizeCallback);

    YgImage image;
    ygCreateImage(&image);

    while (!glfwWindowShouldClose(pWindow)) {
        if (ygSwapchain.recreated) {
            //
        }

        VkCommandBuffer cmd = ygAcquireNextImage();

        // Push descriptor

        // Draw

        ygPresent(cmd, image);

        glfwPollEvents();
    }

    cleanUp();
}