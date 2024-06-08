#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

// uniform block
layout (std140) uniform Common
{
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

out vec2 TexCoords;
out vec4 ParticleColour;

uniform bool sphericalBillboarding;
uniform mat4 model;
uniform vec4 colour;

void main() {
    mat4 viewModel = view * model;

    // First column
    viewModel[0][0] = 1.0f;
    viewModel[0][1] = 0.0f;
    viewModel[0][2] = 0.0f;

    if (sphericalBillboarding) {
        // Second column
        viewModel[1][0] = 0.0f;
        viewModel[1][1] = 1.0f;
        viewModel[1][2] = 0.0f;
    }

    // Third column
    viewModel[2][0] = 0.0f;
    viewModel[2][1] = 0.0f;
    viewModel[2][2] = 1.0f;

    TexCoords = aTexCoords;
    ParticleColour = colour;

    gl_Position = projection * viewModel * vec4(aPos, 1.0);
}