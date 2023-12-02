#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push {
    mat4 model_matrix;
    mat4 normal_matrix;
}push;

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection_matrix;
    mat4 view_matrix;
    vec4 ambient_light_color;
    vec3 light_position;
    vec4 light_color;
} ubo;


void main() {
    vec3 direction_to_light = ubo.light_position - fragPosWorld;
    float attenuation = 1.0f / dot(direction_to_light, direction_to_light); // distance squared

    vec3 light_color = ubo.light_color.xyz * ubo.light_color.w * attenuation; // w is intensity.

    vec3 ambient_light = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w;

    // interpolate might lead to un-normal
    vec3 diffuse_light = light_color * max(dot(normalize(fragNormalWorld), normalize(direction_to_light)), 0);

    outColor = vec4((diffuse_light + ambient_light) * fragColor, 1.0);
}
