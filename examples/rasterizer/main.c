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
    int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);

    // Wait while the window is minimized
    while (width == 0 || height == 0) {
        glfwWaitEvents();
        glfwGetFramebufferSize(pWindow, &width, &height);
    }

    *pWidth = (uint32_t)width;
    *pHeight = (uint32_t)height;
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
}

static void createInstance()
{
    uint32_t instanceExtensionCount;
    const char** ppInstanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);

    gfxCreateInstance(VK_API_VERSION_1_3, instanceExtensionCount, ppInstanceExtensions);
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

static void updateCamera(const GfxBuffer* pCameraBuffer)
{
    CameraMatrices cameraMatrices = {
        .view = mat4_lookat((vec3){0.0f, 0.0f, 5.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}),
        .proj = mat4_perspective(gfxSwapchain.extent.width, gfxSwapchain.extent.height, 0.01f, 100.0f,
                                 30.0f * 3.14159265f / 180.0f),
    };

    gfxCopyBufferFromHost(pCameraBuffer, &cameraMatrices, sizeof(CameraMatrices), 0);
}

// What the draw pass needs to record its commands
typedef struct {
    GfxShader* pShader;
    const GfxBuffer* pVertexBuffer;
    const GfxBuffer* pIndexBuffer;
} DrawPass;

// Called by the render graph while it renders to the color and depth images
static void drawPass(VkCommandBuffer cmd, const GfxAttachment* pAttachment, void* pUserData)
{
    const DrawPass* pDraw = pUserData;

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

    // Set rendering states. Depth writes are on by default.
    gfxCmdSetDefaultStates(cmd, pAttachment, GFX_ARRAY_LEN(vertexBindingDescription), vertexBindingDescription,
                           GFX_ARRAY_LEN(vertexAttributeDescription), vertexAttributeDescription);

    // Bind shader and the resources attached to it
    gfxCmdBindShader(cmd, pDraw->pShader);
    gfxCmdBindResources(cmd, pDraw->pShader);

    // Bind vertex and index buffers
    VkDeviceSize vertexBufferOffset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &pDraw->pVertexBuffer->buffer, &vertexBufferOffset);
    vkCmdBindIndexBuffer(cmd, pDraw->pIndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
}

int main()
{
    createWindow();

    createInstance();

    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(gfxDevice.instance, pWindow, NULL, &surface));

    gfxCreateDevice(GFX_DEFAULT_DEVICE, 0, NULL, NULL, surface);

    gfxCreateSwapchain(2, framebufferSizeCallback);

    // The render graph owns the images the frame is rendered to, and recreates
    // them when the window is resized. They are multisampled, and the graph
    // resolves the color image at the end of the pass.
    GfxRenderGraph graph;
    gfxCreateRenderGraph(&graph);
    gfxAddGraphImage(&graph, &(GfxGraphImageDesc){
                                 .pName = "color",
                                 .format = VK_FORMAT_R8G8B8A8_UNORM,
                                 .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 .samples = gfxGetDeviceSampleCount(),
                                 .clearColor = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}},
                             });
    gfxAddGraphImage(&graph, &(GfxGraphImageDesc){
                                 .pName = "depth",
                                 .format = gfxFindDepthFormat(),
                                 .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                 .samples = gfxGetDeviceSampleCount(),
                             });

    GfxShaderStageDesc shaderStages[] = {
        {.pPath = "shader.vert"},
        {.pPath = "shader.frag"},
    };
    GfxShader shader;
    gfxCreateShader(GFX_ARRAY_LEN(shaderStages), shaderStages, &shader);

    GfxBuffer cameraBuffer;
    gfxCreateBuffer(sizeof(CameraMatrices), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &cameraBuffer);
    updateCamera(&cameraBuffer);

    // The model matrix changes every frame, so every frame in flight gets its own copy
    GfxBuffer modelBuffer;
    gfxCreatePerFrameBuffer(sizeof(mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &modelBuffer);
    float angle = 0.0f;

    GfxTexture texture;
    gfxCreateTextureFromFile(GFX_TEXTURE_2D, VK_FORMAT_R8G8B8A8_UNORM, "texture.png", false, &texture);
    gfxSetTextureMagFilter(&texture, VK_FILTER_NEAREST);

    GfxBuffer vertexBuffer;
    GfxBuffer indexBuffer;
    createVertexAndIndexBuffers(&vertexBuffer, &indexBuffer);

    // Attach resources by the names they have in the shader
    gfxSetShaderBuffer(&shader, "camera", &cameraBuffer);
    gfxSetShaderBuffer(&shader, "mesh", &modelBuffer);
    gfxSetShaderTexture(&shader, "diffuseTexture", &texture);

    // A single pass draws the quad to the color and depth images, and the
    // color image is what gets presented
    DrawPass draw = {
        .pShader = &shader,
        .pVertexBuffer = &vertexBuffer,
        .pIndexBuffer = &indexBuffer,
    };
    gfxAddGraphPass(&graph, &(GfxPassDesc){
                                .pName = "Draw",
                                .outputs = {"color", "depth"},
                                .function = drawPass,
                                .pUserData = &draw,
                            });
    gfxSetGraphOutput(&graph, "color");

    bool reloadKeyDown = false;
    bool screenshotKeyDown = false;

    // Seconds since last game loop
    float delta = 0.0f;
    while (!glfwWindowShouldClose(pWindow)) {
        delta = (float)glfwGetTime();
        glfwSetTime(0.0);

        // Reload shaders when F5 is pressed
        bool reloadKey = glfwGetKey(pWindow, GLFW_KEY_F5) == GLFW_PRESS;
        if (reloadKey && !reloadKeyDown) {
            gfxReloadShader(&shader);
        }
        reloadKeyDown = reloadKey;

        // Wait for shared resources to be available
        gfxWaitForFence();

        // Rotate model. This frame's copy is no longer read by the device.
        angle += delta;
        mat4* model = gfxGetPerFrameData(&modelBuffer);
        *model = mat4_trs_rotate(angle, (vec3){0.0f, 1.0f, 0.0f});

        // If the swapchain changed, the projection matrix should be updated.
        if (gfxSwapchain.recreated) {
            updateCamera(&cameraBuffer);
        }

        // Start new frame. No command buffer means the swapchain was out of
        // date and got recreated, so skip this frame.
        VkCommandBuffer cmd = gfxAcquireNextImage();
        if (!cmd) {
            glfwPollEvents();
            continue;
        }

        gfxCmdExecuteRenderGraph(cmd, &graph);
        gfxPresent(cmd, gfxGetGraphImage(&graph, "color"));

        // Save the presented frame when F12 is pressed
        bool screenshotKey = glfwGetKey(pWindow, GLFW_KEY_F12) == GLFW_PRESS;
        if (screenshotKey && !screenshotKeyDown) {
            gfxSaveImagePNG(gfxGetGraphImage(&graph, "color"), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "screenshot.png");
        }
        screenshotKeyDown = screenshotKey;

        glfwPollEvents();
    }

    gfxDestroyBuffer(&vertexBuffer);
    gfxDestroyBuffer(&indexBuffer);

    gfxDestroyTexture(&texture);

    gfxDestroyBuffer(&cameraBuffer);
    gfxDestroyBuffer(&modelBuffer);

    gfxDestroyShader(&shader);

    gfxDestroyRenderGraph(&graph);

    gfxDestroySwapchain();
    gfxDestroyDevice();
    gfxDestroyInstance();

    glfwTerminate();
}
