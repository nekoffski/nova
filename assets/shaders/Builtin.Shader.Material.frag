// clang-format off
#version 450
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_scalar_block_layout : require


layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 color;
    vec3 position;
    vec3 attenuation;
};

struct DirectionalLight {
    vec4 color;
    vec3 direction;
};

layout (std430, set = 0, binding = 0) uniform GlobalUBO {
    mat4 projection;
    mat4 view;
    mat4 depthMVP;
    vec3 viewPosition;
    int mode;
    vec4 ambientColor;
    PointLight pointLights[5];
    DirectionalLight directionalLights[5];
    int pointLightCount;
    int directionalLightCount;
} globalUBO;

layout (set = 0, binding = 1) uniform sampler2D shadowMap;

layout (set = 1, binding = 0) uniform LocalUBO {
    vec4 diffuseColor;
    float shininess;
} localUBO;

const int diffuseMap  = 0;
const int specularMap = 1;
const int normalMap   = 2;

const float epsilon = 0.00001;

layout (set = 1, binding = 1) uniform sampler2D textures[3];

layout (location = 0) flat in int renderMode; // flat indicates that it's not gonna be interpolated between vertices

layout (location = 1) in struct DTO { 
    vec2 textureCoordinates; 
    vec3 normal;
    vec3 viewPosition;
    vec3 fragmentPosition;
    vec4 ambient;
    vec4 color;
    vec4 tangent;
    vec4 shadowCoord;
} dto;

mat3 TBN;

vec4 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection) {
    vec3 direction = normalize(-light.direction);
    float diffuseFactor = max(dot(normal, direction), 0.0);

    vec3 halfDirection = normalize(viewDirection + direction);
    float specularFactor = pow(max(dot(halfDirection, normal), epsilon), localUBO.shininess);

    vec4 diffuseTextureSample = texture(textures[diffuseMap], dto.textureCoordinates);
    vec4 specularTextureSample = vec4(texture(
        textures[specularMap], dto.textureCoordinates).rgb, diffuseTextureSample.a);

    vec4 ambient = vec4(vec3(localUBO.diffuseColor * dto.ambient), 1.0);
    vec4 diffuse = vec4(vec3(light.color * diffuseFactor), 1.0);
    vec4 specular = vec4(vec3(light.color * specularFactor), 1.0);

    if (renderMode == 0) {
        diffuse *= diffuseTextureSample;
        ambient *= diffuseTextureSample;
        specular *= specularTextureSample;
    }
    return diffuse + ambient + specular;
}

vec4 calculatePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection) {
    vec3 lightDirection = normalize(light.position.xyz - fragmentPosition);
    float diff = max(dot(normal, lightDirection), 0.0);

    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), epsilon), localUBO.shininess);

    float d = length(light.position.xyz - fragmentPosition);
    float attenuation = 1.0 / (light.attenuation.z + light.attenuation.y * d + light.attenuation.x * (d * d));

    vec4 ambient = dto.ambient;
    vec4 diffuse = light.color * diff;
    vec4 specular = light.color * spec;

    if (renderMode == 0) {
        vec4 diffuseTextureSample = texture(textures[diffuseMap], dto.textureCoordinates);
        vec4 specularTextureSample = vec4(texture(
            textures[specularMap], dto.textureCoordinates).rgb, diffuseTextureSample.a);

        diffuse *= diffuseTextureSample;
        ambient *= diffuseTextureSample;
        specular *= specularTextureSample;
    }
    return (ambient + diffuse + specular) * attenuation;
}

void main() { 
    vec3 normal = dto.normal;

    if (renderMode == 0) {
        vec3 tangent = dto.tangent.xyz;
        tangent = tangent - dot(tangent, normal) * normal;
        vec3 bitangent = cross(dto.normal, dto.tangent.xyz) * dto.tangent.w;

        TBN = mat3(tangent, bitangent, normal);
        vec3 localNormal = 2.0 * texture(textures[normalMap], dto.textureCoordinates).rgb - 1.0;
        normal = normalize(TBN * localNormal);
    }

    if (renderMode == 0 || renderMode == 1) {
        vec3 viewDirection = normalize(dto.viewPosition - dto.fragmentPosition);
        outColor = dto.ambient * texture(textures[diffuseMap], dto.textureCoordinates);

        for (int i = 0; i < globalUBO.directionalLightCount; ++i) {
            float bias = 0.05f;
            float visibility = 1.0f;

            vec3 projCoords = dto.shadowCoord.xyz / dto.shadowCoord.w;
            float ss = texture(shadowMap, projCoords.xy).r;
            
            if (ss < projCoords.z - bias) {
                visibility = 0.3f;
            }
            outColor += visibility * calculateDirectionalLight(globalUBO.directionalLights[i], normal, viewDirection);
        }
        
        for (int i = 0; i < globalUBO.pointLightCount; ++i)
            outColor += calculatePointLight(globalUBO.pointLights[i], normal, dto.fragmentPosition, viewDirection);
    } else {
        outColor = vec4(abs(normal), 1.0);
    }
}
   