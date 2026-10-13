// Shared by the shadow and scene shaders, which use the same buffer and push
// constants

layout(set = 0, binding = 0) uniform FrameUniforms {
    mat4 view;
    mat4 proj;
    mat4 lightViewProj;
    vec4 lightDirection; // Towards the light
    vec4 cameraPosition;
} frame;

layout(push_constant) uniform PushConstants {
    mat4 model;
    uint textured;
} pc;
