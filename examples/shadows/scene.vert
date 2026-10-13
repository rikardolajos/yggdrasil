#version 460
#include "common.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoord;

layout(location = 0) out vec3 worldPosition;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 uv;
layout(location = 3) out vec4 lightPosition;

void main()
{
    vec4 world = pc.model * vec4(position, 1.0);

    worldPosition = world.xyz;
    // The quad lies in the xy-plane of its object space
    normal = mat3(pc.model) * vec3(0.0, 0.0, 1.0);
    uv = textureCoord;
    lightPosition = frame.lightViewProj * world;

    gl_Position = frame.proj * frame.view * world;
}
