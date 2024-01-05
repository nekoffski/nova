#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTextureCoordinates;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec4 inTangent;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 projection;
    mat4 view;
}
globalUBO;

layout(location = 0) out vec3 textureCoordinates;

void main() {
    textureCoordinates = inPosition;
    gl_Position = globalUBO.projection * globalUBO.view * vec4(inPosition, 1.0);
}