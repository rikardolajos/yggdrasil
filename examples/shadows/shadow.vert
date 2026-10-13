#version 460
#include "common.glsl"

layout(location = 0) in vec3 position;

// Only depth is written, so the shadow pass has no fragment shader
void main()
{
    gl_Position = frame.lightViewProj * pc.model * vec4(position, 1.0);
}
