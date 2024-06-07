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

uniform vec3 offset;
uniform vec3 scale;
uniform vec4 colour;

void main() {
    vec3 viewScale;
    viewScale.x = length(vec3(view[0][0], view[0][1], view[0][2]));
    viewScale.y = length(vec3(view[1][0], view[1][1], view[1][2]));
    viewScale.z = length(vec3(view[2][0], view[2][1], view[2][2]));

    vec3 viewTranslation = vec3(view[3][0], view[3][1], view[3][2]);

    mat4 billboardView = mat4(1.0);

    billboardView[0][0] = viewScale.x;
    billboardView[1][1] = viewScale.y;
    billboardView[2][2] = viewScale.z;

    billboardView[3] = vec4(viewTranslation, 1.0);

    TexCoords = aTexCoords;
    ParticleColour = colour;

    vec4 worldPos = vec4((aPos * scale) + offset, 1.0);
    gl_Position = projection * billboardView * worldPos;
}