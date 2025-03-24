#define YGGDRASIL_STBI
#define YGGDRASIL_IMPLEMENTATION
#include "yggdrasil.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GLFW/glfw3.h>


GLFWwindow* pWindow;

//
uint8_t shaderModule[] = {0xff};

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

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    pWindow = glfwCreateWindow(1280, 720, "Hello Triangle", NULL, NULL);
    if (!pWindow) {
        glfwTerminate();
        YG_ERROR("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(pWindow);
}

void createInstance()
{
    uint32_t instanceExtensionCount;
    const char** ppInstanceExtensions =
        glfwGetRequiredInstanceExtensions(&instanceExtensionCount);

    ygCreateInstance(VK_API_VERSION_1_3, ppInstanceExtensions,
                     instanceExtensionCount);
}

void createDevice(VkSurfaceKHR surface)
{
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

void createAttachments(YgImage* pColorAttachment, YgImage* pDepthAttachment,
                       YgImage* pResolveAttachment)
{
    ygCreateImage(ygSwapchain.extent.width, ygSwapchain.extent.height, 1,
                  ygGetDeviceSampleCount(), VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_STORAGE_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pColorAttachment);

    ygCreateImage(ygSwapchain.extent.width, ygSwapchain.extent.height, 1,
                  VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_STORAGE_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pDepthAttachment);

    ygCreateImage(ygSwapchain.extent.width, ygSwapchain.extent.height, 1,
                  VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_STORAGE_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pResolveAttachment);
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

    VkSurfaceKHR surface;
    VK_CHECK(
        glfwCreateWindowSurface(ygDevice.instance, pWindow, NULL, &surface));

    createDevice(surface);

    ygCreateSwapchain(2, framebufferSizeCallback);

    YgImage colorAttachment;
    YgImage depthAttachment;
    YgImage resolveAttachment;
    createAttachments(&colorAttachment, &depthAttachment, &resolveAttachment);

    YgPass pass;
    ygCreatePass(&colorAttachment, 1, &depthAttachment, &resolveAttachment, &pass);

    while (!glfwWindowShouldClose(pWindow)) {
        if (ygSwapchain.recreated) {
            ygDestroyImage(&colorAttachment);
            ygDestroyImage(&depthAttachment);
            ygDestroyImage(&resolveAttachment);
            createAttachments(&colorAttachment, &depthAttachment,
                              &resolveAttachment);
            ygRecreatePass(&pass, &colorAttachment, 1, &depthAttachment,
                         &resolveAttachment);
        }

        VkCommandBuffer cmd = ygAcquireNextImage();

        ygBeginPass(
            &pass, cmd,
            (VkClearValue){.color = {{0.0f, 0.0f, 0.0f, 0.0f}},
                           .depthStencil = {.depth = 1.0f, .stencil = 0}},
            VK_ATTACHMENT_LOAD_OP_CLEAR);

        // Push descriptor

        // Draw

        ygEndPass(&pass, cmd);

        ygPresent(cmd, &colorAttachment);

        glfwPollEvents();
    }

    ygDestroyImage(&colorAttachment);
    ygDestroyImage(&depthAttachment);
    ygDestroyImage(&resolveAttachment);

    ygDestroyPass(&pass);

    cleanUp();
}
