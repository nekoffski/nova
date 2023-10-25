// clang-format off

#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTextureCoordinates;
layout (location = 3) in vec4 inColor;
layout (location = 4) in vec4 inTangent;

layout (set = 0, binding = 0) uniform GlobalUBO {
    mat4 projection;
    mat4 view;
    vec3 viewPosition;
    int mode;
    vec4 ambientColor;
} globalUBO;

layout (location = 0) out int renderMode;

layout (location = 1) out struct DTO { 
    vec2 textureCoordinates; 
    vec3 normal;
    vec3 viewPosition;
    vec3 fragmentPosition;
    vec4 ambient;
    vec4 color;
    vec4 tangent;
} dto;

layout (push_constant) uniform pushConstants_t { 
    mat4 model; 
} pushConstants;

void main() {
    dto.textureCoordinates = inTextureCoordinates;
    dto.normal = mat3(pushConstants.model) * inNormal;
    dto.viewPosition = globalUBO.viewPosition;
    dto.fragmentPosition = vec3(pushConstants.model * vec4(inPosition, 1.0));
    dto.ambient = globalUBO.ambientColor;
    dto.color = inColor;
    dto.tangent = vec4(normalize(mat3(pushConstants.model) * inTangent.xyz), inTangent.w);
    renderMode = globalUBO.mode;

    gl_Position = globalUBO.projection * 
        globalUBO.view * pushConstants.model * vec4(inPosition, 1.0);
}
