#version 460
#include "common.glsl"
#include "hit.glsl"

layout(push_constant) uniform PushConstants {
    vec4 lightDirection;
} pc;

layout(location = 0) rayPayloadInEXT RayPayload payload;
layout(location = 1) rayPayloadEXT bool shadowed;

void main()
{
    vec3 position = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

    // The quad lies in the xy-plane of its object space. It is double sided,
    // so the normal is turned towards the ray.
    vec3 normal = normalize(gl_ObjectToWorldEXT * vec4(0.0, 0.0, 1.0, 0.0));
    if (dot(normal, gl_WorldRayDirectionEXT) > 0.0) {
        normal = -normal;
    }

    vec3 albedo;
    if (gl_InstanceCustomIndexEXT == TEXTURED_INSTANCE) {
        albedo = textureLod(diffuseTexture, hitTextureCoord(), 0.0).rgb;
    } else {
        vec2 cell = floor(position.xz);
        albedo = mod(cell.x + cell.y, 2.0) == 0.0 ? vec3(0.8) : vec3(0.4);
    }

    // Trace towards the light with the shadow miss shader. Only the any-hit
    // shader runs on hits, so the parts it cuts out let light through.
    vec3 lightDirection = normalize(pc.lightDirection.xyz);
    shadowed = true;
    traceRayEXT(scene, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xff, 0, 0, 1,
                position + 0.001 * normal, 0.001, lightDirection, 1000.0, 1);

    float diffuse = shadowed ? 0.0 : max(dot(normal, lightDirection), 0.0);
    payload.color = albedo * (0.25 + 0.75 * diffuse);
}
