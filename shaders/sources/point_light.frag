#version 450

layout(location = 0) in vec2 fragOffset;

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection_matrix;
    mat4 view_matrix;
    vec4 ambient_light_color;
    vec3 light_position;
    vec4 light_color;
} ubo;


void main() {
    float dist = sqrt(dot(fragOffset, fragOffset));
    if(dist >= 1.0f) {
        discard;
    }

    outColor = vec4(ubo.light_color.xyz, 1.0f);
}