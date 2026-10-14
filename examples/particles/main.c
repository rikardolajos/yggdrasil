#define GFX_USE_STB_IMAGE_WRITE
#define GFX_IMPLEMENTATION
#include "gfx.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vecmat.h"

#include <GLFW/glfw3.h>

#define PARTICLE_COUNT (256 * 1024)

GLFWwindow* pWindow;

// Matches the Particle struct in the shaders
typedef struct {
    float position[4];
    float velocity[4];
} Particle;

// Matches CameraUniforms in particles.vert
typedef struct {
    mat4 view;
    mat4 proj;
} CameraMatrices;

// Matches the push constants in particles.comp
typedef struct {
    float attractor[3];
    float strength;
    float delta;
    float damping;
    uint32_t particleCount;
} SimulationParameters;

// Matches the push constants in particles.vert
typedef struct {
    float pointSize;
    float speedScale;
} RenderParameters;

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

    pWindow = glfwCreateWindow(1280, 720, "GFX Compute Particles Example", NULL, NULL);
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
    // Points larger than one pixel need largePoints, on top of what yggdrasil
    // enables itself
    VkPhysicalDeviceFeatures2 features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .features =
            {
                .largePoints = VK_TRUE,
            },
    };

    gfxCreateDevice(GFX_DEFAULT_DEVICE, 0, NULL, &features, surface);
}

static float randomFloat()
{
    return (float)rand() / (float)RAND_MAX;
}

// Spread the particles over a thin disc and give them a tangential velocity,
// so that the simulation starts out as a rotating cloud rather than a
// collapsing one
static void seedParticles(const GfxBuffer* pParticleBuffer)
{
    Particle* pParticles = GFX_MALLOC(PARTICLE_COUNT * sizeof *pParticles);

    for (uint32_t i = 0; i < PARTICLE_COUNT; i++) {
        float angle = 6.283185307f * randomFloat();
        float radius = 3.0f + 3.0f * randomFloat();
        float height = 0.5f * (randomFloat() - 0.5f);

        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        // The cross product of the up axis and the position
        pParticles[i] = (Particle){
            .position = {x, height, z, 0.0f},
            .velocity = {0.4f * z, 0.0f, -0.4f * x, 0.0f},
        };
    }

    gfxCopyBufferFromHost(pParticleBuffer, pParticles, PARTICLE_COUNT * sizeof *pParticles, 0);

    GFX_FREE(pParticles);
}

static void cmdSetRenderingStates(VkCommandBuffer cmd, const GfxAttachment* pAttachmentSet)
{
    // The particles are read from the storage buffer by vertex index, so there
    // is no vertex input to describe
    gfxCmdSetDefaultStates(cmd, pAttachmentSet, 0, NULL, 0, NULL);

    vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    vkCmdSetDepthTestEnable(cmd, VK_FALSE);
    vkCmdSetDepthWriteEnable(cmd, VK_FALSE);

    // Blend additively so that dense regions light up
    const VkBool32 blendEnable = VK_TRUE;
    const VkColorBlendEquationEXT additive = {
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };
    gfxDevice.fn.vkCmdSetColorBlendEnableEXT(cmd, 0, 1, &blendEnable);
    gfxDevice.fn.vkCmdSetColorBlendEquationEXT(cmd, 0, 1, &additive);
}

// What the simulation pass needs to record its commands
typedef struct {
    GfxShader* pShader;
    SimulationParameters parameters;
} SimulatePass;

// What the draw pass needs to record its commands
typedef struct {
    GfxShader* pShader;
    RenderParameters parameters;
} DrawPass;

// Advance the simulation. Called by the render graph after the previous
// frame's draw is done with the particles.
static void simulatePass(VkCommandBuffer cmd, const GfxAttachment* pAttachment, void* pUserData)
{
    SimulatePass* pSimulate = pUserData;

    gfxCmdBindShader(cmd, pSimulate->pShader);
    gfxCmdBindResources(cmd, pSimulate->pShader);
    gfxCmdPushConstants(cmd, pSimulate->pShader, 0, sizeof pSimulate->parameters, &pSimulate->parameters);
    gfxCmdDispatch(cmd, pSimulate->pShader, PARTICLE_COUNT, 1, 1);
}

// Draw the particles. Called by the render graph while it renders to the color
// image, after the simulation's writes have been made visible.
static void drawPass(VkCommandBuffer cmd, const GfxAttachment* pAttachment, void* pUserData)
{
    DrawPass* pDraw = pUserData;

    cmdSetRenderingStates(cmd, pAttachment);

    gfxCmdBindShader(cmd, pDraw->pShader);
    gfxCmdBindResources(cmd, pDraw->pShader);
    gfxCmdPushConstants(cmd, pDraw->pShader, 0, sizeof pDraw->parameters, &pDraw->parameters);

    // Draw one point per particle, the vertex shader looks each one up by its
    // vertex index
    vkCmdDraw(cmd, PARTICLE_COUNT, 1, 0, 0);
}

// Edge triggered key press
static bool keyPressed(int key, bool* pWasDown)
{
    bool down = glfwGetKey(pWindow, key) == GLFW_PRESS;
    bool pressed = down && !*pWasDown;
    *pWasDown = down;
    return pressed;
}

int main()
{
    createWindow();

    createInstance();

    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(gfxDevice.instance, pWindow, NULL, &surface));

    createDevice(surface);

    gfxCreateSwapchain(2, framebufferSizeCallback);

    // The render graph owns the image the particles are drawn to, and
    // recreates it when the window is resized
    GfxRenderGraph graph;
    gfxCreateRenderGraph(&graph);
    gfxAddGraphImage(&graph, &(GfxGraphImageDesc){
                                 .pName = "color",
                                 .format = VK_FORMAT_R8G8B8A8_UNORM,
                                 .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 .clearColor = {.float32 = {0.01f, 0.01f, 0.03f, 1.0f}},
                             });

    // One shader advances the simulation, the other draws it
    GfxShaderStageDesc simulationStage = {.pPath = "particles.comp"};
    GfxShader simulation;
    gfxCreateShader(1, &simulationStage, &simulation);

    GfxShaderStageDesc renderStages[] = {
        {.pPath = "particles.vert"},
        {.pPath = "particles.frag"},
    };
    GfxShader render;
    gfxCreateShader(GFX_ARRAY_LEN(renderStages), renderStages, &render);

    // The particles live on the device: the compute shader writes them and the
    // vertex shader reads them, so the host only touches them to seed them
    GfxBuffer particleBuffer;
    gfxCreateBuffer(PARTICLE_COUNT * sizeof(Particle),
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &particleBuffer);
    seedParticles(&particleBuffer);

    // The camera orbits the cloud, so every frame in flight gets its own copy
    GfxBuffer cameraBuffer;
    gfxCreatePerFrameBuffer(sizeof(CameraMatrices), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &cameraBuffer);

    // The particle block is declared without an instance name in both shaders,
    // so it is attached by its block name
    gfxSetShaderBuffer(&simulation, "Particles", &particleBuffer);
    gfxSetShaderBuffer(&render, "Particles", &particleBuffer);
    gfxSetShaderBuffer(&render, "camera", &cameraBuffer);

    SimulatePass simulate = {
        .pShader = &simulation,
        .parameters = {.strength = 20.0f, .damping = 0.15f, .particleCount = PARTICLE_COUNT},
    };
    DrawPass draw = {
        .pShader = &render,
        .parameters = {.pointSize = 2.0f, .speedScale = 0.2f},
    };

    // The particle buffer is in the graph too, so that the simulation runs
    // before the draw that reads it, and after the previous frame's draw
    gfxAddGraphBuffer(&graph, "particles", &particleBuffer);
    gfxAddGraphPass(&graph, &(GfxPassDesc){
                                .pName = "Simulate",
                                .type = GFX_PASS_COMPUTE,
                                .outputs = {"particles"},
                                .function = simulatePass,
                                .pUserData = &simulate,
                            });
    gfxAddGraphPass(&graph, &(GfxPassDesc){
                                .pName = "Draw",
                                .inputs = {"particles"},
                                .outputs = {"color"},
                                .function = drawPass,
                                .pUserData = &draw,
                            });
    gfxSetGraphOutput(&graph, "color");

    GFX_INFO("Simulating %d particles in workgroups of %" PRIu32 ". Space pauses, R resets, F5 reloads shaders and "
             "F12 saves a screenshot.",
             PARTICLE_COUNT, simulation.localSize[0]);

    bool paused = false;
    bool pauseKeyDown = false;
    bool resetKeyDown = false;
    bool reloadKeyDown = false;
    bool screenshotKeyDown = false;
    float time = 0.0f;
    float cameraAngle = 0.0f;

    // Seconds since last game loop
    float delta = 0.0f;
    while (!glfwWindowShouldClose(pWindow)) {
        delta = (float)glfwGetTime();
        glfwSetTime(0.0);

        if (keyPressed(GLFW_KEY_SPACE, &pauseKeyDown)) {
            paused = !paused;
        }

        // Uploading waits for the device, so frames in flight are done with the
        // particles
        if (keyPressed(GLFW_KEY_R, &resetKeyDown)) {
            seedParticles(&particleBuffer);
        }

        if (keyPressed(GLFW_KEY_F5, &reloadKeyDown)) {
            gfxReloadShader(&simulation);
            gfxReloadShader(&render);
        }

        // Wait for shared resources to be available
        gfxWaitForFence();

        // A long frame would make the integration blow up, so the time step is
        // capped
        float step = paused ? 0.0f : GFX_MIN(delta, 1.0f / 30.0f);
        time += step;
        cameraAngle += 0.1f * step;

        // Move the attractor along a slow figure-of-eight, so that the cloud
        // keeps getting stirred
        simulate.parameters.attractor[0] = 4.0f * sinf(0.7f * time);
        simulate.parameters.attractor[1] = 2.0f * sinf(1.4f * time);
        simulate.parameters.attractor[2] = 4.0f * cosf(0.7f * time);
        simulate.parameters.delta = step;

        // This frame's copy of the camera is no longer read by the device
        CameraMatrices* pCamera = gfxGetPerFrameData(&cameraBuffer);
        vec3 cameraPosition = vec3_make(14.0f * sinf(cameraAngle), 6.0f, 14.0f * cosf(cameraAngle));
        pCamera->view = mat4_lookat(cameraPosition, vec3_make(0.0f, 0.0f, 0.0f), vec3_make(0.0f, 1.0f, 0.0f));
        pCamera->proj = mat4_perspective(gfxSwapchain.extent.width, gfxSwapchain.extent.height, 0.1f, 100.0f,
                                         60.0f * 3.14159265f / 180.0f);

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
        if (keyPressed(GLFW_KEY_F12, &screenshotKeyDown)) {
            gfxSaveImagePNG(gfxGetGraphImage(&graph, "color"), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "screenshot.png");
        }

        glfwPollEvents();
    }

    gfxDestroyBuffer(&particleBuffer);
    gfxDestroyBuffer(&cameraBuffer);

    gfxDestroyShader(&simulation);
    gfxDestroyShader(&render);

    gfxDestroyRenderGraph(&graph);

    gfxDestroySwapchain();
    gfxDestroyDevice();
    gfxDestroyInstance();

    glfwTerminate();
}
