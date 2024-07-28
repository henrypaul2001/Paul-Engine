#version 330 core

layout (location = 0) out vec4 FragColour;

in vec2 TexCoords;

uniform samplerCube cubemap;
uniform int faceIndex;

const vec3 faceVectors[6] = vec3[6](
    vec3(1.0, 0.0, 0.0),  // GL_TEXTURE_CUBE_MAP_POSITIVE_X
    vec3(-1.0, 0.0, 0.0), // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    vec3(0.0, 1.0, 0.0),  // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    vec3(0.0, -1.0, 0.0), // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    vec3(0.0, 0.0, 1.0),  // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    vec3(0.0, 0.0, -1.0)  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
);

void main() {
    vec3 direction;

    if (faceIndex == 0) {
        direction = vec3(1.0, TexCoords.y * 2.0 - 1.0, TexCoords.x * 2.0 - 1.0);
    } else if (faceIndex == 1) {
        direction = vec3(-1.0, TexCoords.y * 2.0 - 1.0, -(TexCoords.x * 2.0 - 1.0));
    } else if (faceIndex == 2) {
        direction = vec3(TexCoords.x * 2.0 - 1.0, 1.0, TexCoords.y * 2.0 - 1.0);
    } else if (faceIndex == 3) {
        direction = vec3(TexCoords.x * 2.0 - 1.0, -1.0, -(TexCoords.y * 2.0 - 1.0));
    } else if (faceIndex == 4) {
        direction = vec3(TexCoords.x * 2.0 - 1.0, TexCoords.y * 2.0 - 1.0, 1.0);
    } else if (faceIndex == 5) {
        direction = vec3(-(TexCoords.x * 2.0 - 1.0), TexCoords.y * 2.0 - 1.0, -1.0);
    }

    direction = normalize(direction);
    FragColour = texture(cubemap, direction);
}