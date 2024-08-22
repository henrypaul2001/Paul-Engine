#version 330 core
layout (location = 0) in vec3 aPos;

// uniform block
layout (std140) uniform Common
{
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

out mat4[] Projection;

void main() {
    Projection[0] = projection;

    gl_Position = view * vec4(aPos, 1.0);
}