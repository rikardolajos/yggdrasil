#version 460
#include "common.glsl"

layout(set = 0, binding = 1) uniform sampler2D diffuseTexture;

// The depth the shadow pass rendered, sampled with a comparing sampler
layout(set = 0, binding = 2) uniform sampler2DShadow shadowMap;

layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 lightPosition;

layout(location = 0) out vec4 fragColor;

// How lit a point is, from 0 to 1. Each lookup compares against the shadow map
// and filters the four nearest results, and averaging a 3x3 grid of lookups
// softens the shadow edges further.
float lightVisibility()
{
    vec3 ndc = lightPosition.xyz / lightPosition.w;
    vec2 shadowCoord = ndc.xy * 0.5 + 0.5;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    float visibility = 0.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            visibility += texture(shadowMap, vec3(shadowCoord + vec2(x, y) * texelSize, ndc.z));
        }
    }
    return visibility / 9.0;
}

void main()
{
    // The quads are double sided, so the normal is turned towards the camera
    vec3 n = normalize(normal);
    if (dot(n, frame.cameraPosition.xyz - worldPosition) < 0.0) {
        n = -n;
    }

    vec3 albedo;
    if (pc.textured != 0) {
        albedo = texture(diffuseTexture, uv).rgb;
    } else {
        vec2 cell = floor(worldPosition.xz);
        albedo = mod(cell.x + cell.y, 2.0) == 0.0 ? vec3(0.8) : vec3(0.4);
    }

    vec3 lightDirection = normalize(frame.lightDirection.xyz);
    float diffuse = max(dot(n, lightDirection), 0.0) * lightVisibility();

    fragColor = vec4(albedo * (0.25 + 0.75 * diffuse), 1.0);
}
