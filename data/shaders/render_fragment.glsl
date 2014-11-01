#version 130

in vec3 v_normal;
in vec3 v_real_position;

out vec4 out_color;

uniform vec3 light_pos;

void main() {
    const vec3 eye_pos = vec3(0.0f, 0.0f, 0.0f);
    const float ambiency = 0.2f;
    const float diffuse_effect = 0.5f;
    const float specular_effect = 0.8f;
    const vec3 fragment_color = vec3(1.0f, 1.0f, 1.0f);

    vec3 normal = normalize(v_normal);
    vec3 light_dir = normalize(light_pos - v_real_position);

    float diffuse = dot(normal, light_dir);

    float specular = 0.0f;
    if (diffuse > 0.0f) {
        vec3 eye_dir = normalize(eye_pos - v_real_position);
        vec3 reflect_dir = normalize(reflect(-light_dir, normal));
        specular = dot(eye_dir, reflect_dir);
        specular = max(specular, 0.0);
        specular = pow(specular, 16);
        specular = clamp(specular * specular_effect, 0.0, 1.0);
    }
    diffuse = clamp(diffuse * diffuse_effect, 0.0f, 1.0f);

    float intencity = clamp(diffuse + specular + ambiency, 0.0f, 1.0f);

    out_color = vec4(fragment_color * intencity, 1.0f);
}
