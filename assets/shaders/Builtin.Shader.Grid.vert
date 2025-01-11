// clang-format off

#version 450
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_debug_printf : enable

layout (set = 0, binding = 0) uniform GlobalUBO {
    mat4 view;
    mat4 projection;
} globalUBO;

layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;

vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec3 unprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    vec3 p = gridPlane[gl_VertexIndex].xyz;
    nearPoint = unprojectPoint(p.x, p.y, 0.0, globalUBO.view, globalUBO.projection).xyz;
    farPoint = unprojectPoint(p.x, p.y, 1.0, globalUBO.view, globalUBO.projection).xyz;
    gl_Position = vec4(p, 1.0);
}
