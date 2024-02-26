#version 450

layout(location = 0) in vec2 fragOffset;

layout(location = 0) out vec4 outColor;

struct PointLight{
    vec4 position;  // ignore w
    vec4 color; // w is intensity
};

layout (std140, set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection_matrix;
    mat4 view_matrix;
    mat4 inverse_view_matrix;
    vec4 ambient_light_color;
    vec4 direct_light;
    PointLight point_lights[10];    // can apply Specialization Constants
    int num_lights;
} ubo;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;
} push;

const float M_PI = 3.14159265359;
void main() {
    float dist = sqrt(dot(fragOffset, fragOffset));
    if(dist >= 1.0f) {
        discard;
    }

    outColor = vec4(push.color.xyz, 0.5f * (cos(dist * M_PI) + 1.0f));
}