#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform Push {
    mat4 transform;
    mat4 normal_matrix;
}push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0f, -3.0f, -1.0f));
const float AMBIENT = 0.02f;

void main() {
    gl_Position = push.transform * vec4(position, 1.0f);

    // suitable for situation only using uniform scaling
    // vec3 normal_world_space = normalize(mat3(push.model_matrix) * normal);
    // computational expensive
    // vec3 normal_world_space = mat3(transpose(inverse(push.model_matrix))) * normal;
    vec3 normal_world_space = normalize(mat3(push.normal_matrix) * normal);
    float light_intensity = AMBIENT + max(dot(normal_world_space, DIRECTION_TO_LIGHT), 0);

    fragColor = light_intensity * color;
}
