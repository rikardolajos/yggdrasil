#define GFX_USE_STB_IMAGE
#define GFX_IMPLEMENTATION
#include "yggdrasil.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vecmat.h"

#include <GLFW/glfw3.h>

GLFWwindow* pWindow;

typedef struct {
    mat4 view;
    mat4 proj;
} CameraMatrices;

typedef struct {
    vec3 position;
    vec2 textureCoord;
} Vertex;

static void framebufferSizeCallback(uint32_t* pWidth, uint32_t* pHeight)
{
    *pWidth = 0;
    *pHeight = 0;

    // Handle minimization
    do {
        glfwGetFramebufferSize(pWindow, pWidth, pHeight);
        glfwWaitEvents();
    } while (*pWidth == 0 || *pHeight == 0);
}

static void createWindow()
{
    if (!glfwInit()) {
        GFX_ERROR("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    pWindow = glfwCreateWindow(1280, 720, "GFX Example", NULL, NULL);
    if (!pWindow) {
        glfwTerminate();
        GFX_ERROR("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(pWindow);
}

static void createInstance()
{
    uint32_t instanceExtensionCount;
    const char** ppInstanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);

    gfxCreateInstance(VK_API_VERSION_1_3, instanceExtensionCount, ppInstanceExtensions);
}

static void createDevice(VkSurfaceKHR surface)
{
    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
        VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
    };

    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
        .shaderObject = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features vk13Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &shaderObjectFeatures,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
        .shaderDemoteToHelperInvocation = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 features = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                                          .pNext = &vk13Features,
                                          .features = {
                                              .samplerAnisotropy = VK_TRUE,
                                          }};

    gfxCreateDevice(0, GFX_ARRAY_LEN(deviceExtensions), deviceExtensions, &features, surface);
}

static void createAttachments(GfxImage* pColorAttachment, GfxImage* pDepthAttachment)
{
    gfxCreateImage(gfxSwapchain.extent.width, gfxSwapchain.extent.height, 1, VK_SAMPLE_COUNT_1_BIT,
                   VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pColorAttachment);
    gfxCreateImageView(pColorAttachment, VK_IMAGE_ASPECT_COLOR_BIT);

    VkFormat depthFormat = gfxFindDepthFormat();
    gfxCreateImage(gfxSwapchain.extent.width, gfxSwapchain.extent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat,
                   VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pDepthAttachment);
    gfxCreateImageView(pDepthAttachment, VK_IMAGE_ASPECT_DEPTH_BIT);

    // Transition depth attachment for depth use
    VkImageSubresourceRange depthSubresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                     .baseMipLevel = 0,
                                                     .levelCount = 1,
                                                     .baseArrayLayer = 0,
                                                     .layerCount = 1};
    if (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
        depthSubresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    VkCommandBuffer cmd = gfxCmdBegin();
    gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, pDepthAttachment->image,
                    &depthSubresourceRange);
    gfxCmdEnd(cmd);
}

static void createLayout(GfxLayout* pLayout)
{
    VkDescriptorType types[] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    };

    VkShaderStageFlags stages[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    uint32_t counts[] = {1, 1, 1};

    gfxCreateLayout(GFX_ARRAY_LEN(types), types, stages, counts, 0, NULL, pLayout);
}

static void createVertexAndIndexBuffers(GfxBuffer* pVertexBuffer, GfxBuffer* pIndexBuffer)
{
    Vertex vertices[] = {
        {.position = {-1.0f, -1.0f, 0.0f}, .textureCoord = {0.0f, 0.0f}},
        {.position = {1.0f, -1.0f, 0.0f}, .textureCoord = {1.0f, 0.0f}},
        {.position = {-1.0f, 1.0f, 0.0f}, .textureCoord = {0.0f, 1.0f}},
        {.position = {1.0f, 1.0f, 0.0f}, .textureCoord = {1.0f, 1.0f}},
    };
    uint32_t indices[] = {0, 1, 3, 0, 3, 2};

    size_t verticesSize = GFX_ARRAY_LEN(vertices) * sizeof(Vertex);
    size_t indicesSize = GFX_ARRAY_LEN(indices) * sizeof(uint32_t);

    gfxCreateBuffer(verticesSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pVertexBuffer);
    gfxCreateBuffer(indicesSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pIndexBuffer);

    gfxCopyBufferFromHost(pVertexBuffer, vertices, sizeof(vertices), 0);
    gfxCopyBufferFromHost(pIndexBuffer, indices, sizeof(indices), 0);
}

static void cmdSetRenderingStates(VkCommandBuffer cmd)
{
    const VkVertexInputBindingDescription2EXT vertexBindingDescription[] = {{
        .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        .divisor = 1,
    }};

    const VkVertexInputAttributeDescription2EXT vertexAttributeDescription[] = {
        {
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position),
        },
        {
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, textureCoord),
        },
    };

    gfxCmdSetDefaultStates(cmd, GFX_ARRAY_LEN(vertexBindingDescription), vertexBindingDescription,
                           GFX_ARRAY_LEN(vertexAttributeDescription), vertexAttributeDescription);
}

int main()
{
    createWindow();

    createInstance();

    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(gfxDevice.instance, pWindow, NULL, &surface));

    createDevice(surface);

    gfxCreateSwapchain(2, framebufferSizeCallback);

    GfxImage colorAttachment;
    GfxImage depthAttachment;
    createAttachments(&colorAttachment, &depthAttachment);

    GfxAttachment attachment;
    gfxCreateAttachment(1, &colorAttachment, &depthAttachment, NULL, &attachment);

    GfxLayout layout;
    createLayout(&layout);

    GfxShader vertexShader;
    GfxShader fragmentShader;
    gfxCreateShaderFromFileGLSL("shader.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, &layout,
                                &vertexShader);
    gfxCreateShaderFromFileGLSL("shader.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0, &layout, &fragmentShader);
    gfxBuildLinkedShaders(&vertexShader, &fragmentShader);

    GfxBuffer cameraBuffer;
    gfxCreateBuffer(sizeof(CameraMatrices), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &cameraBuffer);

    CameraMatrices cameraMatrices = {
        .view = mat4_lookat((vec3){0.0f, 0.0f, 5.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}),
        .proj = mat4_perspective(gfxSwapchain.extent.width, gfxSwapchain.extent.height, 0.01f, 100.0f, 30.0f),
    };

    gfxCopyBufferFromHost(&cameraBuffer, &cameraMatrices, sizeof(CameraMatrices), 0);

    GfxBuffer modelBuffer;
    gfxCreateBuffer(sizeof(mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &modelBuffer);
    float angle = 0.0f;
    mat4* model = modelBuffer.pHostMap;
    *model = mat4_trs_rotate(angle, (vec3){0.0f, 1.0f, 0.0f});

    GfxTexture texture;
    gfxCreateTextureFromFile(GFX_TEXTURE_2D, VK_FORMAT_R8G8B8A8_UNORM, "texture.png", false, &texture);

    GfxBuffer vertexBuffer;
    GfxBuffer indexBuffer;
    createVertexAndIndexBuffers(&vertexBuffer, &indexBuffer);

    // Seconds since last game loop
    float delta = 0.0f;
    while (!glfwWindowShouldClose(pWindow)) {
        delta = (float)glfwGetTime();
        glfwSetTime(0.0);

        // Wait for shared resources to be available
        ygWaitForFence();

        // Rotate model
        angle += delta;
        *model = mat4_trs_rotate(angle, (vec3){0.0f, 1.0f, 0.0f});

        // Check if swapchain has been recreated
        if (gfxSwapchain.recreated) {
            gfxDestroyImage(&colorAttachment);
            gfxDestroyImage(&depthAttachment);
            createAttachments(&colorAttachment, &depthAttachment);
            gfxRecreateAttachment(&attachment, 1, &colorAttachment, &depthAttachment, NULL);
        }

        // Start new frame. No command buffer means the swapchain was out of
        // date and got recreated, so skip this frame.
        VkCommandBuffer cmd = gfxAcquireNextImage();
        if (!cmd) {
            glfwPollEvents();
            continue;
        }

        gfxTransitionForColorAttachment(cmd, &colorAttachment);

        VkClearValue clearValue = {.color = {{0.0f, 0.0f, 0.0f, 1.0f}}};
        gfxCmdBeginRendering(cmd, &attachment, clearValue, VK_ATTACHMENT_LOAD_OP_CLEAR);

        // Set rendering states
        cmdSetRenderingStates(cmd);
        vkCmdSetDepthWriteEnable(cmd, VK_TRUE);

        // Push descriptor
        VkWriteDescriptorSet writes[] = {
            gfxGetBufferDescriptor(&cameraBuffer, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_WHOLE_SIZE),
            gfxGetBufferDescriptor(&modelBuffer, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_WHOLE_SIZE),
            gfxGetTextureDescriptor(&texture, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
        };
        VK_LOAD(vkCmdPushDescriptorSetKHR);
        XvkCmdPushDescriptorSetKHR(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout.pipelineLayout, 0,
                                   GFX_ARRAY_LEN(writes), writes);

        // Bind shaders
        gfxCmdBindShader(cmd, &vertexShader);
        gfxCmdBindShader(cmd, &fragmentShader);

        // Bind vertex and index buffers
        VkDeviceSize vertexBufferOffset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer.buffer, &vertexBufferOffset);
        vkCmdBindIndexBuffer(cmd, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        // Draw
        vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);

        // End and present frame
        gfxCmdEndRendering(cmd, &attachment);
        gfxTransitionForBlitting(cmd, &colorAttachment);
        gfxPresent(cmd, &colorAttachment);

        glfwPollEvents();
    }

    gfxDestroyBuffer(&vertexBuffer);
    gfxDestroyBuffer(&indexBuffer);

    gfxDestroyTexture(&texture);

    gfxDestroyBuffer(&cameraBuffer);
    gfxDestroyBuffer(&modelBuffer);

    gfxDestroyShader(&vertexShader);
    gfxDestroyShader(&fragmentShader);

    gfxDestroyLayout(&layout);

    gfxDestroyImage(&colorAttachment);
    gfxDestroyImage(&depthAttachment);

    gfxDestroyAttachment(&attachment);

    gfxDestroySwapchain();
    gfxDestroyDevice();
    gfxDestroyInstance();

    glfwTerminate();
}
