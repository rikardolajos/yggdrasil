#define GFX_USE_STB_IMAGE
#define GFX_USE_STB_IMAGE_WRITE
#define GFX_IMPLEMENTATION
#include "gfx.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vecmat.h"

#include <GLFW/glfw3.h>

GLFWwindow* pWindow;

// Matches CameraUniforms in raygen.rgen
typedef struct {
    vec4 position;
    vec4 right;
    vec4 up;
    vec4 forward;
} CameraUniforms;

typedef struct {
    float position[3];
    float textureCoord[2];
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

    pWindow = glfwCreateWindow(1280, 720, "GFX Ray Tracer Example", NULL, NULL);
    if (!pWindow) {
        glfwTerminate();
        GFX_ERROR("Failed to create GLFW window");
    }
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
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
    };

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .rayTracingPipeline = VK_TRUE,
    };

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        .pNext = &rayTracingPipelineFeatures,
        .accelerationStructure = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features vk13Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &accelerationStructureFeatures,
        .synchronization2 = VK_TRUE,
    };

    VkPhysicalDeviceVulkan12Features vk12Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &vk13Features,
        .bufferDeviceAddress = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 features = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                                          .pNext = &vk12Features,
                                          .features = {
                                              .samplerAnisotropy = VK_TRUE,
                                          }};

    gfxCreateDevice(GFX_DEFAULT_DEVICE, GFX_ARRAY_LEN(deviceExtensions), deviceExtensions, &features, surface);
}

static void createOutputImage(GfxImage* pImage)
{
    VkExtent3D extent = {.width = gfxSwapchain.extent.width, .height = gfxSwapchain.extent.height, .depth = 1};

    gfxCreateImage(extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                   VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0, VK_IMAGE_TYPE_2D,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pImage);
    gfxCreateImageView(pImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
}

static void updateCamera(const GfxBuffer* pCameraBuffer)
{
    vec3 position = {0.0f, 0.8f, 5.0f};
    vec3 target = {0.0f, -0.4f, 0.0f};
    float fov = 0.8f;

    vec3 forward = vec3_normalize(vec3_sub(target, position));
    vec3 right = vec3_normalize(vec3_cross(forward, (vec3){0.0f, 1.0f, 0.0f}));
    vec3 up = vec3_cross(right, forward);

    float tanHalfFov = tanf(0.5f * fov);
    float aspect = (float)gfxSwapchain.extent.width / (float)gfxSwapchain.extent.height;
    right = vec3_scale(tanHalfFov * aspect, right);
    up = vec3_scale(tanHalfFov, up);

    CameraUniforms camera = {
        .position = {position.x, position.y, position.z, 1.0f},
        .right = {right.x, right.y, right.z, 0.0f},
        .up = {up.x, up.y, up.z, 0.0f},
        .forward = {forward.x, forward.y, forward.z, 0.0f},
    };

    gfxCopyBufferFromHost(pCameraBuffer, &camera, sizeof camera, 0);
}

static void createGeometry(GfxBuffer* pVertexBuffer, GfxBuffer* pIndexBuffer)
{
    Vertex vertices[] = {
        {.position = {-1.0f, -1.0f, 0.0f}, .textureCoord = {0.0f, 0.0f}},
        {.position = {1.0f, -1.0f, 0.0f}, .textureCoord = {1.0f, 0.0f}},
        {.position = {-1.0f, 1.0f, 0.0f}, .textureCoord = {0.0f, 1.0f}},
        {.position = {1.0f, 1.0f, 0.0f}, .textureCoord = {1.0f, 1.0f}},
    };
    uint32_t indices[] = {0, 1, 3, 0, 3, 2};

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                               VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                               VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

    gfxCreateBuffer(sizeof(vertices), usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pVertexBuffer);
    gfxCreateBuffer(sizeof(indices), usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pIndexBuffer);

    gfxCopyBufferFromHost(pVertexBuffer, vertices, sizeof(vertices), 0);
    gfxCopyBufferFromHost(pIndexBuffer, indices, sizeof(indices), 0);
}

// The floor is the quad laid flat and scaled up, below the spinning quad
static VkAccelerationStructureInstanceKHR floorInstance(const GfxAccelerationStructure* pBLAS)
{
    mat4 transform = mat4_mul(
        mat4_trs_translate((vec3){0.0f, -1.2f, 0.0f}),
        mat4_mul(mat4_trs_rotate(-1.5707963f, (vec3){1.0f, 0.0f, 0.0f}), mat4_trs_scale((vec3){6.0f, 6.0f, 1.0f})));
    return gfxAccelerationStructureInstance(pBLAS, transform.array, 0);
}

static VkAccelerationStructureInstanceKHR quadInstance(const GfxAccelerationStructure* pBLAS, float angle)
{
    mat4 transform = mat4_trs_rotate(angle, (vec3){0.0f, 1.0f, 0.0f});
    return gfxAccelerationStructureInstance(pBLAS, transform.array, 1);
}

int main()
{
    createWindow();

    createInstance();

    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(gfxDevice.instance, pWindow, NULL, &surface));

    createDevice(surface);

    gfxCreateSwapchain(2, framebufferSizeCallback);

    GfxImage outputImage;
    createOutputImage(&outputImage);

    GfxShaderStageDesc shaderStages[] = {
        {.pPath = "raygen.rgen"},      {.pPath = "miss.rmiss"},   {.pPath = "shadow.rmiss"},
        {.pPath = "closesthit.rchit"}, {.pPath = "anyhit.rahit"},
    };
    GfxShader shader;
    gfxCreateShader(GFX_ARRAY_LEN(shaderStages), shaderStages, &shader);

    // Camera and shadow rays are traced from the closest hit shader
    GfxRayTracingPipeline pipeline;
    gfxCreateRayTracingPipeline(&shader, 2, &pipeline);

    GfxBuffer cameraBuffer;
    gfxCreateBuffer(sizeof(CameraUniforms), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &cameraBuffer);
    updateCamera(&cameraBuffer);

    GfxTexture texture;
    gfxCreateTextureFromFile(GFX_TEXTURE_2D, VK_FORMAT_R8G8B8A8_UNORM, "texture.png", false, &texture);
    gfxSetTextureMagFilter(&texture, VK_FILTER_NEAREST);

    GfxBuffer vertexBuffer;
    GfxBuffer indexBuffer;
    createGeometry(&vertexBuffer, &indexBuffer);

    // The geometry is not opaque, so that the any-hit shader runs and can cut
    // out parts of it
    GfxGeometryDesc geometry = {
        .pVertexBuffer = &vertexBuffer,
        .vertexStride = sizeof(Vertex),
        .vertexCount = 4,
        .pIndexBuffer = &indexBuffer,
        .indexCount = 6,
    };
    GfxAccelerationStructure blas;
    gfxCreateBLAS(1, &geometry, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, &blas);

    // Both instances use the same geometry. The quad is moved every frame, so
    // the TLAS allows updates.
    float angle = 0.0f;
    VkAccelerationStructureInstanceKHR instances[] = {floorInstance(&blas), quadInstance(&blas, angle)};
    GfxAccelerationStructure tlas;
    gfxCreateTLAS(GFX_ARRAY_LEN(instances), instances,
                  VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR |
                      VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
                  &tlas);

    // Attach resources by the names they have in the shaders
    gfxSetShaderAccelerationStructure(&shader, "scene", &tlas);
    gfxSetShaderBuffer(&shader, "vertices", &vertexBuffer);
    gfxSetShaderBuffer(&shader, "indices", &indexBuffer);
    gfxSetShaderTexture(&shader, "diffuseTexture", &texture);
    gfxSetShaderImage(&shader, "outputImage", &outputImage, VK_IMAGE_LAYOUT_GENERAL);
    gfxSetShaderBuffer(&shader, "camera", &cameraBuffer);

    float lightDirection[4] = {0.6f, 1.0f, 0.4f, 0.0f};

    bool reloadKeyDown = false;
    bool screenshotKeyDown = false;

    // Seconds since last game loop
    float delta = 0.0f;
    while (!glfwWindowShouldClose(pWindow)) {
        delta = (float)glfwGetTime();
        glfwSetTime(0.0);

        // Reload shaders when F5 is pressed. The pipeline is rebuilt when it
        // is bound next.
        bool reloadKey = glfwGetKey(pWindow, GLFW_KEY_F5) == GLFW_PRESS;
        if (reloadKey && !reloadKeyDown) {
            gfxReloadShader(&shader);
        }
        reloadKeyDown = reloadKey;

        // Check if swapchain has been recreated
        if (gfxSwapchain.recreated) {
            gfxDestroyImage(&outputImage);
            createOutputImage(&outputImage);
            updateCamera(&cameraBuffer);
        }

        // Start new frame. No command buffer means the swapchain was out of
        // date and got recreated, so skip this frame.
        VkCommandBuffer cmd = gfxAcquireNextImage();
        if (!cmd) {
            glfwPollEvents();
            continue;
        }

        // Spin the quad
        angle += delta;
        instances[1] = quadInstance(&blas, angle);
        gfxCmdUpdateTLAS(cmd, &tlas, GFX_ARRAY_LEN(instances), instances);

        gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                        VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_2_SHADER_WRITE_BIT,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, outputImage.image, NULL);

        gfxCmdBindRayTracingPipeline(cmd, &pipeline);
        gfxCmdBindResources(cmd, &shader);
        gfxCmdPushConstants(cmd, &shader, 0, sizeof(lightDirection), lightDirection);
        gfxCmdTraceRays(cmd, &pipeline, outputImage.width, outputImage.height, 1);

        gfxImageBarrier(cmd, VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_2_SHADER_WRITE_BIT,
                        VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, outputImage.image, NULL);

        gfxPresent(cmd, &outputImage);

        // Save the presented frame when F12 is pressed
        bool screenshotKey = glfwGetKey(pWindow, GLFW_KEY_F12) == GLFW_PRESS;
        if (screenshotKey && !screenshotKeyDown) {
            gfxSaveImagePNG(&outputImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "screenshot.png");
        }
        screenshotKeyDown = screenshotKey;

        glfwPollEvents();
    }

    gfxDestroyAccelerationStructure(&tlas);
    gfxDestroyAccelerationStructure(&blas);

    gfxDestroyBuffer(&vertexBuffer);
    gfxDestroyBuffer(&indexBuffer);

    gfxDestroyTexture(&texture);

    gfxDestroyBuffer(&cameraBuffer);

    gfxDestroyRayTracingPipeline(&pipeline);
    gfxDestroyShader(&shader);

    gfxDestroyImage(&outputImage);

    gfxDestroySwapchain();
    gfxDestroyDevice();
    gfxDestroyInstance();

    glfwTerminate();
}
