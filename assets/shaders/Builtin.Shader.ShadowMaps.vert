// clang-format off

#version 450
#extension GL_EXT_scalar_block_layout : require

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTextureCoordinates;
layout (location = 3) in vec4 inColor;
layout (location = 4) in vec4 inTangent;

layout (std430, set = 0, binding = 0) uniform GlobalUBO {
    mat4 depthMVP;
} globalUBO;

layout (push_constant) uniform pushConstants_t { 
    mat4 model; 
} pushConstants;

void main() {
    gl_Position = globalUBO.depthMVP * pushConstants.model * vec4(inPosition, 1.0);
}
