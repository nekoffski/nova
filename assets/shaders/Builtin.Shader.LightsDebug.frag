// clang-format off
#version 450

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform pushConstants_t { 
    mat4 model; 
    vec4 color;
} pushConstants;

void main() { 
    outColor = pushConstants.color;
}
   