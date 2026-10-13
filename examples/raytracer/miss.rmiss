#version 460
#include "common.glsl"

layout(location = 0) rayPayloadInEXT RayPayload payload;

void main()
{
    float t = 0.5 * (gl_WorldRayDirectionEXT.y + 1.0);
    payload.color = mix(vec3(1.0), vec3(0.5, 0.7, 1.0), t);
}
