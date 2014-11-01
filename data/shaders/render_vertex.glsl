#version 130

uniform mat4 world_projection;
uniform mat4 world;
in vec3 position;
in vec3 normal;

out vec3 v_normal;
out vec3 v_real_position;

void main() {
    v_normal = (world * vec4(normal, 0.0)).xyz;
    v_real_position = (world * vec4(position, 1.0)).xyz;
    gl_Position = world_projection * vec4(position, 1.0);
}
