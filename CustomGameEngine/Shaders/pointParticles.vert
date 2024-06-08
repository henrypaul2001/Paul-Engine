#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aScale;
layout (location = 2) in vec4 aColour; 

// uniform block
layout (std140) uniform Common
{
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

out vec2 Scale;
out vec4 Colour;

void main() {
    Scale = aScale;
    Colour = aColour;
    gl_Position = projection * view * vec4(aPos, 1.0);
}