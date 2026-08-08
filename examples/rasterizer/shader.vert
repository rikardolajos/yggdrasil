#version 460

layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 0, binding = 1) uniform MeshUniforms {
    mat4 model;
} mesh;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTextureCoord;

layout(location = 0) out vec2 texCoord;

void main() {
    texCoord = vertexTextureCoord;
    gl_Position = camera.proj * camera.view * mesh.model * vec4(vertexPosition, 1.0);
}
