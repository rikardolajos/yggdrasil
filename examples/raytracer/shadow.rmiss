#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 1) rayPayloadInEXT bool shadowed;

// A shadow ray that misses everything reaches the light
void main()
{
    shadowed = false;
}
