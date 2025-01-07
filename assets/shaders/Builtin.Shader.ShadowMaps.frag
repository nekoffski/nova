// clang-format off
#version 450
#extension GL_EXT_scalar_block_layout : require

layout (std430, set = 0, binding = 0) uniform GlobalUBO {
    mat4 projection;
    mat4 view;
    vec3 viewPosition;
} globalUBO;


void main() {}
   