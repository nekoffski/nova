// clang-format off
#version 450

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform LocalUBO {
    vec4 diffuseColor;
    float shininess;
} localUBO;

const int diffuseMap  = 0;
const int specularMap = 1;
const int normalMap   = 2;

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
} dto;

mat3 TBN;

struct DirectionalLight {
    vec3 direction;
    vec4 color;
};

struct PointLight {
    vec3 position;
    vec4 color;
    
    // attenuation factors - ax^2 + bx + c
    float c; 
    float b; 
    float a; 
};

DirectionalLight light = {
    vec3(-0.57735, -0.57735, -0.57735),
    vec4(0.8, 0.8, 0.8, 1.0)
};

PointLight pointLights[2] = {
    {
        vec3(-5.5, 0.0, -5.5), vec4(0.0, 1.0, 0.0, 1.0), 1.0, 0.35,0.44
    }, {
        vec3(5.5, 0.0, -5.5), vec4(1.0, 0.0, 0.0, 1.0), 1.0, 0.35, 0.44
    }
};

vec4 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection) {
    float diffuseFactor = max(dot(normal, -light.direction), 0.0);

    vec3 halfDirection = normalize(viewDirection - light.direction);
    float specularFactor = pow(max(dot(halfDirection, normal), 0.0), localUBO.shininess);

    vec4 diffuseTextureSample = texture(textures[diffuseMap], dto.textureCoordinates);
    vec4 specularTextureSample = vec4(texture(
        textures[specularMap], dto.textureCoordinates).rgb, diffuseTextureSample.a);

    vec4 ambient = vec4(vec3(localUBO.diffuseColor * dto.ambient), diffuseTextureSample.a);
    vec4 diffuse = vec4(vec3(light.color * diffuseFactor), diffuseTextureSample.a);
    vec4 specular = vec4(vec3(light.color * specularFactor), diffuseTextureSample.a);

    if (renderMode == 0) {
        diffuse *= diffuseTextureSample;
        ambient *= diffuseTextureSample;
        specular *= specularTextureSample;
    }
    return diffuse + ambient + specular;
}

vec4 calculatePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection) {
    vec3 lightDirection = normalize(light.position - fragmentPosition);
    float diff = max(dot(normal, lightDirection), 0.0);

    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), localUBO.shininess);

    float d = length(light.position - fragmentPosition);
    float attenuation = 1.0 / (light.c + light.b * d + light.a * (d * d));

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
    vec3 tangent = dto.tangent.xyz;
    tangent = tangent - dot(tangent, normal) * normal;
    vec3 bitangent = cross(dto.normal, dto.tangent.xyz) * dto.tangent.w;

    TBN = mat3(tangent, bitangent, normal);
    vec3 localNormal = 2.0 * texture(textures[normalMap], dto.textureCoordinates).rgb - 1.0;
    normal = normalize(TBN * localNormal);

    if (renderMode == 0 || renderMode == 1) {
        vec3 viewDirection = normalize(dto.viewPosition - dto.fragmentPosition);
        outColor = calculateDirectionalLight(light, normal, viewDirection);
        for (int i = 0; i < 2; ++i)
            outColor += calculatePointLight(pointLights[i], normal, dto.fragmentPosition, viewDirection);
    } else {
        outColor = vec4(abs(normal), 1.0);
    }
}
   
