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

void createAttachments(YgImage* pColorAttachment, YgImage* pDepthAttachment)
{
    ygCreateImage(ygSwapchain.extent.width, ygSwapchain.extent.height, 1,
                  VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pColorAttachment);

    VkFormat depthFormat = ygFindDepthFormat();
    ygCreateImage(ygSwapchain.extent.width, ygSwapchain.extent.height, 1,
                  VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pDepthAttachment);

    // Transition depth attachment for depth use
    VkImageSubresourceRange depthSubresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1};
    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
        depthFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
        depthSubresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    VkCommandBuffer cmd = ygCmdBegin();
    ygImageBarrier(cmd, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                   VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                   VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                   pDepthAttachment->image, &depthSubresourceRange);
    ygCmdEnd(cmd);
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
    createAttachments(&colorAttachment, &depthAttachment);

    YgPass pass;
    ygCreatePass(&colorAttachment, 1, &depthAttachment, NULL, &pass);

    while (!glfwWindowShouldClose(pWindow)) {
        if (ygSwapchain.recreated) {
            ygDestroyImage(&colorAttachment);
            ygDestroyImage(&depthAttachment);
            createAttachments(&colorAttachment, &depthAttachment);
            ygRecreatePass(&pass, &colorAttachment, 1, &depthAttachment, NULL);
        }

        VkCommandBuffer cmd = ygAcquireNextImage();

        ygTransitionForColorAttachment(cmd, &colorAttachment);

        ygBeginPass(
            &pass, cmd,
            (VkClearValue){.color = {{0.0f, 0.0f, 0.0f, 0.0f}},
                           .depthStencil = {.depth = 1.0f, .stencil = 0}},
            VK_ATTACHMENT_LOAD_OP_CLEAR);


        // Push descriptor

        // Draw

        ygEndPass(&pass, cmd);

        ygTransitionForBlitting(cmd, &colorAttachment);

        ygPresent(cmd, &colorAttachment);

        glfwPollEvents();
    }

    ygDestroyImage(&colorAttachment);
    ygDestroyImage(&depthAttachment);

    ygDestroyPass(&pass);

    cleanUp();
}
