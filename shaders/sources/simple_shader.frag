#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push {
    mat4 model_matrix;
    mat4 normal_matrix;
}push;

struct PointLight{
    vec4 position;  // ignore w
    vec4 color; // w is intensity
};

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection_matrix;
    mat4 view_matrix;
    mat4 inverse_view_matrix;
    vec4 ambient_light_color;
    PointLight point_lights[10];    // can apply Specialization Constants
    int num_lights;
} ubo;


void main() {
    vec3 diffuse_light = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w;
    vec3 specular_light = vec3(0.0f);
    vec3 surface_normal = normalize(fragNormalWorld);

    vec3 camera_pos_world = ubo.inverse_view_matrix[3].xyz;
    vec3 view_direction = normalize(camera_pos_world - fragPosWorld);

    for(int i = 0; i < ubo.num_lights; i++) {
        PointLight light = ubo.point_lights[i];
        vec3 direction_to_light = light.position.xyz - fragPosWorld;
        float attenuation = 1.0f / dot(direction_to_light, direction_to_light); // distance squared

        direction_to_light = normalize(direction_to_light);

        float cos_angle_incidence = max(dot(surface_normal, direction_to_light), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation; // w is intensity.

        diffuse_light += intensity * cos_angle_incidence;

        // specular lighting
        vec3 half_angle = normalize(direction_to_light + view_direction);
        float binn_term = dot(surface_normal, half_angle);
        binn_term = clamp(binn_term, 0, 1);
        binn_term = pow(binn_term, 128);
        specular_light += intensity * binn_term;
    }

    outColor = vec4((diffuse_light + specular_light) * fragColor, 1.0f);
}
