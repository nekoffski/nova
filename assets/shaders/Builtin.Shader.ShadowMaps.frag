// clang-format off
#version 450
#extension GL_EXT_scalar_block_layout : require


layout (location = 0) out vec4 outColor;


layout (std430, set = 0, binding = 0) uniform GlobalUBO {
    mat4 projection;
    mat4 view;
    vec3 viewPosition;
} globalUBO;


void main() { 
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
   