#version 330 core
layout (location = 0) in vec3 aPos;

// uniform block
layout (std140) uniform Common
{
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

uniform vec3 translation;

void main() {
    vec3 worldPos = translation + aPos;
    gl_Position = projection * view * vec4(worldPos, 1.0);
}