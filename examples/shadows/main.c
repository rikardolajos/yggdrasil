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

#define SHADOW_MAP_SIZE 2048

GLFWwindow* pWindow;

// Matches FrameUniforms in common.glsl
typedef struct {
    mat4 view;
    mat4 proj;
    mat4 lightViewProj;
    vec4 lightDirection;
    vec4 cameraPosition;
} FrameUniforms;

// Matches PushConstants in common.glsl
typedef struct {
    mat4 model;
    uint32_t textured;
} ObjectConstants;

// Size of the push constants in the shaders, which end at the last member
#define OBJECT_CONSTANTS_SIZE (sizeof(mat4) + sizeof(uint32_t))

typedef struct {
    float position[3];
    float textureCoord[2];
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

    pWindow = glfwCreateWindow(1280, 720, "GFX Shadow Mapping Example", NULL, NULL);
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
    };

    VkPhysicalDeviceFeatures2 features = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                                          .pNext = &vk13Features,
                                          .features = {
                                              .samplerAnisotropy = VK_TRUE,
                                          }};

    gfxCreateDevice(GFX_DEFAULT_DEVICE, GFX_ARRAY_LEN(deviceExtensions), deviceExtensions, &features, surface);
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

    gfxCreateBuffer(sizeof(vertices), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pVertexBuffer);
    gfxCreateBuffer(sizeof(indices), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pIndexBuffer);

    gfxCopyBufferFromHost(pVertexBuffer, vertices, sizeof(vertices), 0);
    gfxCopyBufferFromHost(pIndexBuffer, indices, sizeof(indices), 0);
}

// The camera and the light, written every frame since the aspect ratio follows
// the window
static void updateFrameUniforms(FrameUniforms* pFrame)
{
    vec3 cameraPosition = vec3_make(0.0f, 0.8f, 5.0f);
    pFrame->view = mat4_lookat(cameraPosition, vec3_make(0.0f, -0.4f, 0.0f), vec3_make(0.0f, 1.0f, 0.0f));
    pFrame->proj = mat4_perspective(gfxSwapchain.extent.width, gfxSwapchain.extent.height, 0.1f, 100.0f, 0.8f);
    pFrame->cameraPosition = vec4_make(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);

    // A directional light, seen from far along its direction with an
    // orthographic projection that covers the floor
    vec3 lightDirection = vec3_normalize(vec3_make(0.6f, 1.0f, 0.4f));
    mat4 lightView =
        mat4_lookat(vec3_scale(12.0f, lightDirection), vec3_make(0.0f, 0.0f, 0.0f), vec3_make(0.0f, 1.0f, 0.0f));
    mat4 lightProj = mat4_orthographic(8.0f, 8.0f, 0.1f, 30.0f);
    pFrame->lightViewProj = mat4_mul(lightProj, lightView);
    pFrame->lightDirection = vec4_make(lightDirection.x, lightDirection.y, lightDirection.z, 0.0f);
}

// The floor and the spinning quad, drawn from the same geometry
typedef struct {
    GfxShader* pShadowShader;
    GfxShader* pSceneShader;
    const GfxBuffer* pVertexBuffer;
    const GfxBuffer* pIndexBuffer;
    ObjectConstants objects[2];
} Scene;

static void setStates(VkCommandBuffer cmd, const GfxAttachment* pAttachment)
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

    gfxCmdSetDefaultStates(cmd, pAttachment, GFX_ARRAY_LEN(vertexBindingDescription), vertexBindingDescription,
                           GFX_ARRAY_LEN(vertexAttributeDescription), vertexAttributeDescription);
}

static void drawObjects(VkCommandBuffer cmd, const Scene* pScene, GfxShader* pShader)
{
    gfxCmdBindShader(cmd, pShader);
    gfxCmdBindResources(cmd, pShader);

    VkDeviceSize vertexBufferOffset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &pScene->pVertexBuffer->buffer, &vertexBufferOffset);
    vkCmdBindIndexBuffer(cmd, pScene->pIndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);

    for (uint32_t i = 0; i < GFX_ARRAY_LEN(pScene->objects); i++) {
        gfxCmdPushConstants(cmd, pShader, 0, OBJECT_CONSTANTS_SIZE, &pScene->objects[i]);
        vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
    }
}

// Render the scene's depth as seen from the light. The pass only writes the
// shadow map, so the render graph renders it without color attachments.
static void shadowPass(VkCommandBuffer cmd, const GfxAttachment* pAttachment, void* pUserData)
{
    const Scene* pScene = pUserData;

    setStates(cmd, pAttachment);

    // Push the depth away from the light a little, so that surfaces do not
    // shadow themselves. Away is towards smaller depths with reversed depth.
#ifdef GFX_REVERSE_Z
    const float biasSign = -1.0f;
#else
    const float biasSign = 1.0f;
#endif
    vkCmdSetDepthBiasEnable(cmd, VK_TRUE);
    vkCmdSetDepthBias(cmd, biasSign * 1.25f, 0.0f, biasSign * 1.75f);

    drawObjects(cmd, pScene, pScene->pShadowShader);
}

// Render the scene from the camera. The render graph has made the shadow map
// readable by the time this runs.
static void scenePass(VkCommandBuffer cmd, const GfxAttachment* pAttachment, void* pUserData)
{
    const Scene* pScene = pUserData;

    setStates(cmd, pAttachment);
    drawObjects(cmd, pScene, pScene->pSceneShader);
}

int main()
{
    createWindow();

    createInstance();

    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(gfxDevice.instance, pWindow, NULL, &surface));

    createDevice(surface);

    gfxCreateSwapchain(2, framebufferSizeCallback);

    // The shadow map has a fixed size and is both rendered to and sampled. The
    // color and depth images follow the window.
    GfxRenderGraph graph;
    gfxCreateRenderGraph(&graph);
    GfxImage* pShadowMap =
        gfxAddGraphImage(&graph, &(GfxGraphImageDesc){
                                     .pName = "shadowMap",
                                     .format = VK_FORMAT_D32_SFLOAT,
                                     .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                     .width = SHADOW_MAP_SIZE,
                                     .height = SHADOW_MAP_SIZE,
                                 });
    gfxAddGraphImage(&graph, &(GfxGraphImageDesc){
                                 .pName = "color",
                                 .format = VK_FORMAT_R8G8B8A8_UNORM,
                                 .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 .clearColor = {.float32 = {0.6f, 0.75f, 1.0f, 1.0f}},
                             });
    gfxAddGraphImage(&graph, &(GfxGraphImageDesc){
                                 .pName = "depth",
                                 .format = gfxFindDepthFormat(),
                                 .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                             });

    // The shadow shader has no fragment stage, depth is all it produces
    GfxShaderStageDesc shadowStages[] = {{.pPath = "shadow.vert"}};
    GfxShader shadowShader;
    gfxCreateShader(GFX_ARRAY_LEN(shadowStages), shadowStages, &shadowShader);

    GfxShaderStageDesc sceneStages[] = {{.pPath = "scene.vert"}, {.pPath = "scene.frag"}};
    GfxShader sceneShader;
    gfxCreateShader(GFX_ARRAY_LEN(sceneStages), sceneStages, &sceneShader);

    // The camera follows the window size, so every frame in flight gets its own copy
    GfxBuffer frameBuffer;
    gfxCreatePerFrameBuffer(sizeof(FrameUniforms), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &frameBuffer);

    GfxTexture texture;
    gfxCreateTextureFromFile(GFX_TEXTURE_2D, VK_FORMAT_R8G8B8A8_UNORM, "texture.png", false, &texture);
    gfxSetTextureMagFilter(&texture, VK_FILTER_NEAREST);

    // Compares against the shadow map, with linear filtering for softer edges
    VkSampler shadowSampler = gfxCreateSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, true);

    GfxBuffer vertexBuffer;
    GfxBuffer indexBuffer;
    createGeometry(&vertexBuffer, &indexBuffer);

    // Attach resources by the names they have in the shaders
    gfxSetShaderBuffer(&shadowShader, "frame", &frameBuffer);
    gfxSetShaderBuffer(&sceneShader, "frame", &frameBuffer);
    gfxSetShaderTexture(&sceneShader, "diffuseTexture", &texture);
    gfxSetShaderImageSampler(&sceneShader, "shadowMap", pShadowMap, shadowSampler, VK_IMAGE_LAYOUT_UNDEFINED);

    Scene scene = {
        .pShadowShader = &shadowShader,
        .pSceneShader = &sceneShader,
        .pVertexBuffer = &vertexBuffer,
        .pIndexBuffer = &indexBuffer,
    };

    // The floor is the quad laid flat and scaled up, below the spinning quad
    scene.objects[0].model = mat4_mul(mat4_trs_translate(vec3_make(0.0f, -1.2f, 0.0f)),
                                      mat4_mul(mat4_trs_rotate(-1.5707963f, vec3_make(1.0f, 0.0f, 0.0f)),
                                               mat4_trs_scale(vec3_make(6.0f, 6.0f, 1.0f))));
    scene.objects[1].textured = 1;

    // The scene pass reads the shadow map, so the graph runs the shadow pass first
    gfxAddGraphPass(&graph, &(GfxPassDesc){
                                .pName = "Scene",
                                .inputs = {"shadowMap"},
                                .outputs = {"color", "depth"},
                                .function = scenePass,
                                .pUserData = &scene,
                            });
    gfxAddGraphPass(&graph, &(GfxPassDesc){
                                .pName = "Shadow",
                                .outputs = {"shadowMap"},
                                .function = shadowPass,
                                .pUserData = &scene,
                            });
    gfxSetGraphOutput(&graph, "color");

    bool reloadKeyDown = false;
    bool screenshotKeyDown = false;
    float angle = 0.0f;

    // Seconds since last game loop
    float delta = 0.0f;
    while (!glfwWindowShouldClose(pWindow)) {
        delta = (float)glfwGetTime();
        glfwSetTime(0.0);

        // Reload shaders when F5 is pressed
        bool reloadKey = glfwGetKey(pWindow, GLFW_KEY_F5) == GLFW_PRESS;
        if (reloadKey && !reloadKeyDown) {
            gfxReloadShader(&shadowShader);
            gfxReloadShader(&sceneShader);
        }
        reloadKeyDown = reloadKey;

        // Wait for shared resources to be available
        gfxWaitForFence();

        // Spin the quad
        angle += delta;
        scene.objects[1].model = mat4_trs_rotate(angle, vec3_make(0.0f, 1.0f, 0.0f));

        // This frame's copy is no longer read by the device
        updateFrameUniforms(gfxGetPerFrameData(&frameBuffer));

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

    gfxDestroySampler(shadowSampler);
    gfxDestroyTexture(&texture);

    gfxDestroyBuffer(&frameBuffer);

    gfxDestroyShader(&shadowShader);
    gfxDestroyShader(&sceneShader);

    gfxDestroyRenderGraph(&graph);

    gfxDestroySwapchain();
    gfxDestroyDevice();
    gfxDestroyInstance();

    glfwTerminate();
}
