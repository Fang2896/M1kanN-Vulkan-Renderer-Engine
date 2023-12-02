#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragPosWorld;
layout (location = 2) out vec3 fragNormalWorld;

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
    vec4 position_world = push.model_matrix * vec4(position, 1.0f);
    gl_Position = ubo.projection_matrix * ubo.view_matrix * position_world;

    fragNormalWorld = normalize(mat3(push.normal_matrix) * normal);
    fragPosWorld = position_world.xyz;
    fragColor = color;
}
