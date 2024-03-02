#version 450

// reference: https://github.com/PacktPublishing/Mastering-Graphics-Programming-with-Vulkan

struct PointLight{
    vec4 position;  // ignore w
    vec4 color; // w is intensity
};

uint MaterialFeatures_ColorTexture     = 1 << 0;
uint MaterialFeatures_NormalTexture    = 1 << 1;
uint MaterialFeatures_RoughnessTexture = 1 << 2;
uint MaterialFeatures_OcclusionTexture = 1 << 3;
uint MaterialFeatures_EmissiveTexture =  1 << 4;
uint MaterialFeatures_TangentVertexAttribute = 1 << 5;
uint MaterialFeatures_TexcoordVertexAttribute = 1 << 6;

layout (std140, set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection_matrix;
    mat4 view_matrix;
    mat4 inverse_view_matrix;
    vec4 ambient_light_color;
    vec4 direct_light;
    PointLight point_lights[10];    // can apply Specialization Constants
    int num_lights;
} globalUbo;

// for each object or vertex
layout(std140, set = 1, binding = 0) uniform MaterialUbo {
    vec4 base_color_factor;
    mat4 model_matrix;
    mat4 inverse_model_matrix;

    vec3  emissive_factor;
    float metallic_factor;

    float roughness_factor;
    float occlusion_factor;
    uint  flags;
}materialUbo;

layout(location=0) in vec3 position;
layout(location=1) in vec4 tangent;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 texCoord0;

layout (location = 0) out vec2 vTexcoord0;
layout (location = 1) out vec3 vNormalWorld;
layout (location = 2) out vec4 vTangentWorld;
layout (location = 3) out vec4 vPositionWorld;


void main() {
    vPositionWorld = materialUbo.model_matrix * vec4(position, 1);
    gl_Position = globalUbo.projection_matrix * globalUbo.view_matrix * vPositionWorld;

    if ( ( materialUbo.flags & MaterialFeatures_TexcoordVertexAttribute ) != 0 ) {
        vTexcoord0 = texCoord0;
    }
    vNormalWorld = mat3( materialUbo.inverse_model_matrix ) * normal;

    if ( ( materialUbo.flags & MaterialFeatures_TangentVertexAttribute ) != 0 ) {
        vTangentWorld = tangent;
    }
}