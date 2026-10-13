#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 color;
};

layout(set = 0, binding = 0) uniform accelerationStructureEXT scene;

// Every vertex is a position followed by a texture coordinate
layout(set = 0, binding = 1, std430) readonly buffer Vertices {
    float data[];
} vertices;

layout(set = 0, binding = 2, std430) readonly buffer Indices {
    uint data[];
} indices;

layout(set = 0, binding = 3) uniform sampler2D diffuseTexture;

// Custom index of the textured instance, the floor has 0
const uint TEXTURED_INSTANCE = 1;
